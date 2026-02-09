// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FactoryWarehouseSubsystem.generated.h"

class UFactoryItemData;

/**
 * 유저의 창고를 관리하는 서브시스템
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryWarehouseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void StoreItem(UFactoryItemData* ItemData, const int32 Amount);
	bool TryConsumeItem(UFactoryItemData* ItemData, const int32 Amount);
	int32 GetItemAmount(UFactoryItemData* ItemData) const;
	
private:
	UPROPERTY()
	TMap<TObjectPtr<const UFactoryItemData>, int32> WarehouseInventory;
	int32 MaxItemAmount = 8000;
};
