// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/FactoryInventoryComponent.h"


UFactoryInventoryComponent::UFactoryInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UFactoryInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeInventory();
}

void UFactoryInventoryComponent::InitializeInventory()
{
	InventorySlots.SetNum(MaxItemSlotCount);
}

int32 UFactoryInventoryComponent::AddItemToTargetSlot(int32 SlotIndex, FName ItemID, int32 Amount)
{
	if (!InventorySlots.IsValidIndex(SlotIndex) || Amount <= 0 || ItemID == NAME_None) return 0;
	
	FFactoryInventorySlot& TargetSlot = InventorySlots[SlotIndex];
	
	if (TargetSlot.IsEmpty())
	{
		TargetSlot.ItemID = ItemID;
		int32 ClampedAmount = FMath::Clamp(Amount, 0, FFactoryInventorySlot::MaxCapacity);
		TargetSlot.Amount = ClampedAmount;
		return ClampedAmount;
	}
	
	if (TargetSlot.ItemID == ItemID)
	{
		int32 AvailableSpace = TargetSlot.GetAvailableSpace();
		if (AvailableSpace <= 0) return 0; // 슬롯이 이미 가득 찬 경우
		
		int32 AmountToAdd = FMath::Min(Amount, AvailableSpace);
		TargetSlot.Amount += AmountToAdd;
		return AmountToAdd;
	}
	
	return 0;
}

int32 UFactoryInventoryComponent::AutoAddItem(FName ItemID, int32 Amount)
{
	if (Amount <= 0 || ItemID == NAME_None) return 0;
    
	int32 RemainingAmount = Amount;

	// 기존에 같은 아이템이 있는 덜 찬 슬롯을 찾아 채워넣기
	for (int i = 0; i < InventorySlots.Num(); i++)
	{
		if (InventorySlots[i].ItemID == ItemID && !InventorySlots[i].IsFull())
		{
			int32 Added = AddItemToTargetSlot(i, ItemID, RemainingAmount);
			RemainingAmount -= Added;
			if (RemainingAmount <= 0) return Amount; // 다 넣었으면 종료
		}
	}
    
	// 그래도 남았다면 완전 빈 슬롯을 찾아 넣기
	for (int i = 0; i < InventorySlots.Num(); i++)
	{
		if (InventorySlots[i].IsEmpty())
		{
			int32 Added = AddItemToTargetSlot(i, ItemID, RemainingAmount);
			RemainingAmount -= Added;
			if (RemainingAmount <= 0) return Amount;
		}
	}
    
	return Amount - RemainingAmount; // 최종적으로 인벤토리에 들어간 총 개수 반환
}

bool UFactoryInventoryComponent::RemoveItemFromTargetSlot(int32 SlotIndex, int32 Amount)
{
	if (!InventorySlots.IsValidIndex(SlotIndex) || Amount <= 0) return false;
	
	FFactoryInventorySlot& TargetSlot = InventorySlots[SlotIndex];
	if (TargetSlot.IsEmpty()) return false;
	if (TargetSlot.Amount < Amount) return false;
	
	TargetSlot.Amount -= Amount;
	TargetSlot.Amount = FMath::Clamp(TargetSlot.Amount, 0, FFactoryInventorySlot::MaxCapacity);
	if (TargetSlot.Amount == 0)
	{
		TargetSlot.Clear();
	}
	
	return true;
}

bool UFactoryInventoryComponent::AutoRemoveItem(FName ItemID, int32 Amount)
{
	if (Amount <= 0 || ItemID == NAME_None) return false;
	
	if (GetTotalItemAmount(ItemID) < Amount) return false;
	
	int32 RemainingAmount = Amount;
	
	for (int i = InventorySlots.Num() - 1; i >= 0; i--)
	{
		if (InventorySlots[i].ItemID == ItemID && !InventorySlots[i].IsEmpty())
		{
			int32 AmountToRemove = FMath::Min(RemainingAmount, InventorySlots[i].Amount);
			RemoveItemFromTargetSlot(i, AmountToRemove);
			RemainingAmount -= AmountToRemove;
			if (RemainingAmount <= 0) break; // 다 제거했으면 종료
		}
	}
	
	return true;
}

int32 UFactoryInventoryComponent::GetTotalItemAmount(FName ItemID) const
{
	int32 TotalItemAmount = 0;
	
	for (const FFactoryInventorySlot& Slot : InventorySlots)
	{
		if (Slot.ItemID == ItemID)
		{
			TotalItemAmount += Slot.Amount;
		}
	}
	
	return TotalItemAmount;
}

void UFactoryInventoryComponent::SortInventory()
{
	TMap<FName, int32> ItemCountMap;
	
	for (int i = 0; i < InventorySlots.Num(); i++)
	{
		if (InventorySlots[i].IsEmpty()) continue;
		ItemCountMap.FindOrAdd(InventorySlots[i].ItemID, InventorySlots[i].Amount) += InventorySlots[i].Amount;
		InventorySlots[i].Clear();
	}
	
	int32 SlotIndex = 0;
	for (const auto& Pair : ItemCountMap)
	{
		FName ItemID = Pair.Key;
		int32 TotalAmount = Pair.Value;
		
		while (TotalAmount > 0 && SlotIndex < InventorySlots.Num())
		{
			int32 AmountToAdd = FMath::Min(TotalAmount, FFactoryInventorySlot::MaxCapacity);
			InventorySlots[SlotIndex].ItemID = ItemID;
			InventorySlots[SlotIndex].Amount = AmountToAdd;
			TotalAmount -= AmountToAdd;
			SlotIndex++;
		}
		
		if (SlotIndex >= InventorySlots.Num()) break; // 인벤토리가 가득 찬 경우
	}
}




