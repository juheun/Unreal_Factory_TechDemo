// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FactoryPoolSubsystem.generated.h"

class UFactoryItemData;
class AFactoryItemVisual;
/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryPoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	AFactoryItemVisual* GetItemVisualFromPool(const UFactoryItemData* ItemData, const FVector& Location, const FRotator& Rotation);
	void ReturnItemVisualToPool(AFactoryItemVisual* ItemVisual);
	
private:
	UPROPERTY()
	TArray<TObjectPtr<AFactoryItemVisual>> InactiveItemVisuals;
};
