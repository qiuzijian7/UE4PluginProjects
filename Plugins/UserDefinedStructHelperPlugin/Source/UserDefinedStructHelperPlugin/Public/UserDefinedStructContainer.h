// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UserDefinedStructContainer.generated.h"

USTRUCT(BlueprintType)
struct FUserDefinedStructContainer
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		UScriptStruct* ScriptStruct;

	uint8* StructMemory;

	FUserDefinedStructContainer()
		: StructMemory(nullptr)
	{}

	bool IsValid() const
	{
		return ScriptStruct && StructMemory;
	}

	void Destroy(UScriptStruct* ActualStruct);
	void Create(UScriptStruct* ActualStruct);

	~FUserDefinedStructContainer();

	bool Serialize(FArchive& Ar);

	bool Identical(const FUserDefinedStructContainer* Other, uint32 PortFlags) const;

	void AddStructReferencedObjects(class FReferenceCollector& Collector) const;

	FUserDefinedStructContainer& operator=(const FUserDefinedStructContainer& Other);

private:
	FUserDefinedStructContainer(const FUserDefinedStructContainer&);
};

template<>
struct TStructOpsTypeTraits<FUserDefinedStructContainer> : public TStructOpsTypeTraitsBase
{
	enum
	{
		WithZeroConstructor = true,
		WithCopy = true,
		WithIdentical = true,
		WithAddStructReferencedObjects = true,
		WithSerializer = true,
		//WithPostSerialize =true
	};
};