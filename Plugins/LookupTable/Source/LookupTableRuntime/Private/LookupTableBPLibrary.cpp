// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "LookupTable.h"
#include "LookupTableBPLibrary.h"

ULookupTableBPLibrary::ULookupTableBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

float ULookupTableBPLibrary::LookupTableSampleFunction(float Param)
{
	return -1;
}

