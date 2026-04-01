// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/FactoryDeveloperSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FactoryPoolSubsystem.generated.h"

class UFactoryItemData;
class AFactoryItemVisual;

USTRUCT()
struct FFactoryPoolStack
{
	GENERATED_BODY();
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> InactiveItems;
};

UCLASS()
class FACTORYTECHDEMO_API UFactoryPoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	template<typename T>
	T* GetItemFromPool(EFactoryPoolType Type, const FVector& Location, const FRotator& Rotation)
	{
		const UFactoryDeveloperSettings* DeveloperSettings = GetDefault<UFactoryDeveloperSettings>();
		TSubclassOf<AActor> TargetClass = DeveloperSettings->GetPoolClass(Type);
		
		if (TargetClass == nullptr) return nullptr;
		
		return GetItemFromPool<T>(TargetClass, Location, Rotation);
	}
	template<typename T>
	T* GetItemFromPool(TSubclassOf<AActor> ClassType, const FVector& Location, const FRotator& Rotation)
	{
		if (!ClassType) return nullptr;
		return Cast<T>(Internal_GetItem(ClassType, Location, Rotation));
	}

	void ReturnItemToPool(AActor* Actor);
	
private:
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FFactoryPoolStack> PoolMap;
	
	AActor* Internal_GetItem(TSubclassOf<AActor> ClassType, const FVector& Location, const FRotator& Rotation);
};
