// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "UserDefinedStructContainer.h"
#include "DelegateCombinations.h"
#include "UserDefinedStructHelperPluginBPLibrary.generated.h"


DEFINE_LOG_CATEGORY_STATIC(LogUserDefinedStruct,Log,All);

#define LOCTEXT_NAMESPACE "UserDefinedStruct"
/* 
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu. 
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/

USTRUCT()
struct FCustomStruct
{
	GENERATED_USTRUCT_BODY()

		FCustomStruct() { }
};



namespace 
{
	bool GetFriendlyNameFromPropertyName(const FName Name, FName& FriendlyName, FName FriendlyNameWithUniqueIndex)
	{
		const FString NameStr = Name.ToString();
		const int32 GuidStrLen = 32;
		if (NameStr.Len() > (GuidStrLen + 1))
		{
			const int32 UnderscoreIndex = NameStr.Len() - GuidStrLen - 1;
			if (TCHAR('_') == NameStr[UnderscoreIndex])
			{
				FString FriendlyNameWithUniqueIndexStr = NameStr.Left(UnderscoreIndex);
				FriendlyNameWithUniqueIndex = FName(*FriendlyNameWithUniqueIndexStr);
				int32 Index = 0;
				if (FriendlyNameWithUniqueIndexStr.FindLastChar(TCHAR('_'),Index))
				{
					FriendlyName = FName(*FriendlyNameWithUniqueIndexStr.Left(Index));
				}
				
				return true;
			}
		}
		return false;
	}

	template <class T> T* FindFieldWithFriendlyName(const UStruct* Owner, FName FriendlyFieldName)
	{
		// We know that a "none" field won't exist in this Struct
		if (FriendlyFieldName.IsNone())
		{
			return nullptr;
		}

		// Search by comparing FNames (INTs), not strings
		for (TFieldIterator<T>It(Owner); It; ++It)
		{
			FName FriendlyName, FriendlyNameWithUniqueId;
			if (GetFriendlyNameFromPropertyName(It->GetFName(), FriendlyName, FriendlyNameWithUniqueId)) 			
			{
				if (FriendlyName == FriendlyFieldName || FriendlyNameWithUniqueId == FriendlyFieldName)
				{
					return *It;
				}		
			}
		}

		// If we didn't find it, return no field
		return nullptr;
	}

	
	/**
	* Returns a reference to the named property value data in the given container.
	*/
	template<typename T>
	T* GetPropertyValuePtrByName(const UStruct* InStruct, void* InContainer, FString PropertyName, int32 ArrayIndex, UProperty*& OutProperty)
	{
		T* ValuePtr = NULL;

		// Extract the vector ptr recursively using the property name	
		{
			UProperty* Prop = FindFieldWithFriendlyName<UProperty>(InStruct, FName(*PropertyName));
			if (Prop != NULL)
			{
				if (UArrayProperty* ArrayProp = Cast<UArrayProperty>(Prop))
				{
					check(ArrayIndex != INDEX_NONE);

					// Property is an array property, so make sure we have a valid index specified
					FScriptArrayHelper_InContainer ArrayHelper(ArrayProp, InContainer);
					if (ArrayHelper.IsValidIndex(ArrayIndex))
					{
						ValuePtr = (T*)ArrayHelper.GetRawPtr(ArrayIndex);
					}
				}
				else
				{
					// Property is a vector property, so access directly
					ValuePtr = Prop->ContainerPtrToValuePtr<T>(InContainer);
				}

				OutProperty = Prop;
			}
		}

		return ValuePtr;
	}

	/**
	* Returns the value of the property with the given name in the given Actor instance.
	*/
	template<typename T>
	T GetPropertyValueByName(const UStruct* InStruct, void* InContainer, FString PropertyName, int32 PropertyIndex)
	{
		T Value;
		UProperty* DummyProperty = NULL;
		if (T* ValuePtr = GetPropertyValuePtrByName<T>(InStruct, InContainer, PropertyName, PropertyIndex, DummyProperty))
		{
			Value = *ValuePtr;
		}
		return Value;
	}

	/**
	* Sets the property with the given name in the given Actor instance to the given value.
	*/
	template<typename T>
	void SetPropertyValueByName(UObject* Object, FString PropertyName, int32 PropertyIndex, const T& InValue, UProperty*& OutProperty)
	{
		if (T* ValuePtr = GetPropertyValuePtrByName<T>(Object->GetClass(), Object, PropertyName, PropertyIndex, OutProperty))
		{
			*ValuePtr = InValue;
		}
	}
}


UCLASS()
class UUserDefinedStructHelperPluginBPLibrary : public UBlueprintFunctionLibrary
{

		GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "StructContainer", meta = (CustomStructureParam = "CustomStruct"))
	static bool GetUserDefinedStructFromContainer(const FUserDefinedStructContainer& StructContainer, FCustomStruct& CustomStruct);

	DECLARE_FUNCTION(execGetUserDefinedStructFromContainer)
	{
		PARAM_PASSED_BY_REF(StructContainer, UStructProperty, FUserDefinedStructContainer);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentProperty = nullptr;

		Stack.StepCompiledIn<UStructProperty>(NULL);
		void* DstStructAddr = Stack.MostRecentPropertyAddress;
		auto DstStructProperty = Cast<UStructProperty>(Stack.MostRecentProperty);

		bool bResult = false;
		if (DstStructAddr && DstStructProperty && StructContainer.IsValid())
		{
			if (StructContainer.ScriptStruct == DstStructProperty->Struct)
			{
				StructContainer.ScriptStruct->CopyScriptStruct(DstStructAddr, StructContainer.StructMemory);
				bResult = true;
			}
		}

		P_FINISH;

		*(bool*)RESULT_PARAM = bResult;
	}

	UFUNCTION(BlueprintCallable, CustomThunk, Category = "StructContainer", meta = (CustomStructureParam = "CustomStruct"))
	static void SetUserDefinedStructInConainer(UPARAM(ref) FUserDefinedStructContainer& StructContainer, const FCustomStruct& CustomStruct);

	DECLARE_FUNCTION(execSetUserDefinedStructInConainer)
	{
		PARAM_PASSED_BY_REF(StructContainer, UStructProperty, FUserDefinedStructContainer);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentProperty = nullptr;

		Stack.StepCompiledIn<UStructProperty>(NULL);
		void* SrcStructAddr = Stack.MostRecentPropertyAddress;
		auto SrcStructProperty = Cast<UStructProperty>(Stack.MostRecentProperty);

		StructContainer.Destroy(StructContainer.ScriptStruct);

		if (SrcStructAddr && SrcStructProperty)
		{
			StructContainer.ScriptStruct = SrcStructProperty->Struct;
			// TODO: CHECK compatibility 
			StructContainer.Create(StructContainer.ScriptStruct);
			StructContainer.ScriptStruct->CopyScriptStruct(StructContainer.StructMemory, SrcStructAddr);

			//bool a = GetPropertyValueByName<bool>(SrcStructProperty->Struct, StructContainer.StructMemory, "Test_Bool", 0);
			//int32 b = GetPropertyValueByName<int32>(SrcStructProperty->Struct, StructContainer.StructMemory, "Test_Int", 0);
			//UKismetSystemLibrary::PrintString(this, UKismetSystemLibrary::Conv_IntToString(a));
			//UE_LOG(LogUserDefinedStruct, Log, TEXT("[qiu]%d"), b);
		}

		P_FINISH;
	}

	UFUNCTION(BlueprintCallable, CustomThunk, meta = (DefaultToSelf = "Object", ArrayParm = "CustomStruct"), Category = "Utilities|Time")
	static void SortUserDefinedStructArray(const TArray<int32>& CustomStruct, UObject* Object, FName FunctionName);
	
	DECLARE_FUNCTION(execSortUserDefinedStructArray)
	{
		
		Stack.MostRecentProperty = nullptr;
		Stack.StepCompiledIn<UArrayProperty>(NULL);
		void* ArrayAddr = Stack.MostRecentPropertyAddress;
		UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Stack.MostRecentProperty);
		if (!ArrayProperty)
		{
			Stack.bArrayContextFailed = true;
			return;
		}
		P_GET_OBJECT(UObject, OwnerObject);
		P_GET_PROPERTY(UNameProperty, FunctionName);
		if (!OwnerObject)
		{
			return;
		}
		UFunction* const Func = OwnerObject->FindFunction(FunctionName);
		//two input param and one output
		if (!Func || Func->NumParms !=3 )
		{
			return;
		}
		

		P_FINISH;
		P_NATIVE_BEGIN;
		Generic_SortUserDefinedStructArray(ArrayAddr, ArrayProperty, OwnerObject, Func);
		P_NATIVE_END;
	};

	static void Generic_SortUserDefinedStructArray(void* TargetArray, const UArrayProperty* ArrayProp,UObject* OwnerObject,UFunction* SortFunc);
};
#undef LOCTEXT_NAMESPACE