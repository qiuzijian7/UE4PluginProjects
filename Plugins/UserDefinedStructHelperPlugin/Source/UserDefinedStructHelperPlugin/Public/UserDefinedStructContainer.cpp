// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UserDefinedStructHelperPluginPrivatePCH.h"
#include "UserDefinedStructContainer.h"

void FUserDefinedStructContainer::Destroy(UScriptStruct* ActualStruct)
{
	if (ActualStruct && StructMemory)
	{
		ActualStruct->DestroyStruct(StructMemory);
	}

	if (StructMemory)
	{
		FMemory::Free(StructMemory);
		StructMemory = nullptr;
	}
}

FUserDefinedStructContainer::~FUserDefinedStructContainer()
{
	ensure(ScriptStruct || !StructMemory);
	Destroy(ScriptStruct);
}

FUserDefinedStructContainer& FUserDefinedStructContainer::operator=(const FUserDefinedStructContainer& Other)
{
	if (this != &Other)
	{
		Destroy(ScriptStruct);

		ScriptStruct = Other.ScriptStruct;

		if (Other.IsValid())
		{
			Create(ScriptStruct);
			ScriptStruct->CopyScriptStruct(StructMemory, Other.StructMemory);
		}
	}

	return *this;
}

void FUserDefinedStructContainer::Create(UScriptStruct* ActualStruct)
{
	check(NULL == StructMemory);
	StructMemory = (uint8*)FMemory::Malloc(ActualStruct->GetStructureSize());
	ActualStruct->InitializeStruct(StructMemory);
}

bool FUserDefinedStructContainer::Serialize(FArchive& Ar)
{
	auto OldStruct = ScriptStruct;
	Ar << ScriptStruct;
	bool bValidBox = IsValid();
	Ar << bValidBox;

	if (Ar.IsLoading())
	{
		if (OldStruct != ScriptStruct)
		{
			Destroy(OldStruct);
		}
		if (ScriptStruct && !StructMemory && bValidBox)
		{
			Create(ScriptStruct);
		}
	}

	ensure(bValidBox || !IsValid());
	if (IsValid() && bValidBox)
	{
		ScriptStruct->SerializeItem(Ar, StructMemory, nullptr);
	}

	return true;
}

bool FUserDefinedStructContainer::Identical(const FUserDefinedStructContainer* Other, uint32 PortFlags) const
{
	if (!Other)
	{
		return false;
	}

	if (ScriptStruct != Other->ScriptStruct)
	{
		return false;
	}

	if (!ScriptStruct)
	{
		return true;
	}

	if (!StructMemory && !Other->StructMemory)
	{
		return true;
	}

	return ScriptStruct->CompareScriptStruct(StructMemory, Other->StructMemory, PortFlags);
}

void FUserDefinedStructContainer::AddStructReferencedObjects(class FReferenceCollector& Collector) const
{
	Collector.AddReferencedObject(const_cast<FUserDefinedStructContainer*>(this)->ScriptStruct);
	if (ScriptStruct && StructMemory)
	{
		FSimpleObjectReferenceCollectorArchive ObjectReferenceCollector(nullptr, Collector);
		ScriptStruct->SerializeBin(ObjectReferenceCollector, StructMemory);
	}
}