// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UserDefinedStructHelperPluginPrivatePCH.h"
#include "UserDefinedStructHelperPluginBPLibrary.h"


bool UUserDefinedStructHelperPluginBPLibrary::GetUserDefinedStructFromContainer(const FUserDefinedStructContainer& StructContainer, FCustomStruct& CustomStruct)
{
	check(0);
	return false;
}

void UUserDefinedStructHelperPluginBPLibrary::SetUserDefinedStructInConainer(FUserDefinedStructContainer& StructContainer, const FCustomStruct& CustomStruct)
{
	check(0);
}

void UUserDefinedStructHelperPluginBPLibrary::SortUserDefinedStructArray(const TArray<int32>& CustomStruct, UObject* Object, FName FunctionName)
{
	check(0);
}

void UUserDefinedStructHelperPluginBPLibrary::Generic_SortUserDefinedStructArray(void* TargetArray, const UArrayProperty* ArrayProp, UObject* OwnerObject, UFunction* SortRuleFunc)
{
	if (!SortRuleFunc || !OwnerObject || !TargetArray)
	{
		return;
	}

	UBoolProperty* ReturnParam = Cast<UBoolProperty>(SortRuleFunc->GetReturnProperty());
	if(!ReturnParam)
	{
		return;
	}

	//begin Sort Array
	{
		FScriptArrayHelper ArrayHelper(ArrayProp, TargetArray);
		UProperty* InnerProp = ArrayProp->Inner;
		const int32 Len = ArrayHelper.Num();
		const int32 PropertySize = InnerProp->ElementSize * InnerProp->ArrayDim;
		uint8* Parameters = (uint8*)FMemory::Malloc(PropertySize * 2 + 1);
		for(int32 i = 0; i < Len; i++)
		{
			for (int32 j = 0; j < Len - i - 1; j++)
			{			
				FMemory::Memzero(Parameters, PropertySize * 2 + 1);
				InnerProp->CopyCompleteValueFromScriptVM(Parameters, ArrayHelper.GetRawPtr(j));
				InnerProp->CopyCompleteValueFromScriptVM(Parameters + PropertySize, ArrayHelper.GetRawPtr(j + 1));
				OwnerObject->ProcessEvent(SortRuleFunc, Parameters);
				if (ReturnParam && ReturnParam->GetPropertyValue(Parameters + PropertySize * 2))
				{
					ArrayHelper.SwapValues(j, j + 1);
				}			
			}		
		}
		FMemory::Free(Parameters);
	}
	//end Sort Array

}


