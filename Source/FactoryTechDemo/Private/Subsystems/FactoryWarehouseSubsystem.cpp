// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/FactoryWarehouseSubsystem.h"

int32 UFactoryWarehouseSubsystem::AddItem(const UFactoryItemData* ItemData, const int32 Amount)
{
	if (!ItemData) return 0;
	
	int32& StoredAmount = WarehouseInventory.FindOrAdd(ItemData);
	StoredAmount = FMath::Clamp(StoredAmount + Amount, 0, MaxItemAmount);
	
	OnWarehouseItemChanged.Broadcast(ItemData, StoredAmount);
	return StoredAmount;
}

bool UFactoryWarehouseSubsystem::TryRemoveItem(const UFactoryItemData* ItemData, const int32 Amount)
{
	if (!ItemData || Amount <= 0) return false;
	
	int32* StoredAmount = WarehouseInventory.Find(ItemData);
	if (StoredAmount == nullptr || *StoredAmount < Amount)
	{
		return false;
	}
	*StoredAmount -= Amount;
	OnWarehouseItemChanged.Broadcast(ItemData, *StoredAmount);
	return true;
}

int32 UFactoryWarehouseSubsystem::GetItemAmount(const UFactoryItemData* ItemData) const
{
	if (!ItemData) return 0;
	
	const int32* StoredAmount = WarehouseInventory.Find(ItemData);
	return StoredAmount ? *StoredAmount : 0;
}
