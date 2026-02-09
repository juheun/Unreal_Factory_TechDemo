// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/FactoryWarehouseSubsystem.h"

void UFactoryWarehouseSubsystem::StoreItem(UFactoryItemData* ItemData, const int32 Amount)
{
	if (!ItemData) return;
	
	int32& StoredAmount = WarehouseInventory.FindOrAdd(ItemData);
	
	StoredAmount = FMath::Clamp(StoredAmount + Amount, 0, MaxItemAmount);
}

bool UFactoryWarehouseSubsystem::TryConsumeItem(UFactoryItemData* ItemData, const int32 Amount)
{
	if (!ItemData) return false;
	
	int32* StoredAmount = WarehouseInventory.Find(ItemData);
	
	if (StoredAmount == nullptr || *StoredAmount < Amount)
	{
		return false;
	}
	*StoredAmount -= Amount;
	return true;
}

int32 UFactoryWarehouseSubsystem::GetItemAmount(UFactoryItemData* ItemData) const
{
	if (!ItemData) return 0;
	
	const int32* StoredAmount = WarehouseInventory.Find(ItemData);
	if (StoredAmount == nullptr)
	{
		return 0;
	}
	return *StoredAmount;
}
