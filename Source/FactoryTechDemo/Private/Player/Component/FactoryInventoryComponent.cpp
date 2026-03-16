// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/FactoryInventoryComponent.h"

#include "Items/FactoryItemData.h"

UFactoryInventoryComponent::UFactoryInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFactoryInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeInventory();
}

void UFactoryInventoryComponent::InitializeInventory()
{
	InventorySlots.SetNum(MaxItemSlotCount);
}

FFactorySlot UFactoryInventoryComponent::GetSlotData(int32 Index) const
{
	if (InventorySlots.IsValidIndex(Index))
	{
		return InventorySlots[Index];
	}
	return FFactorySlot(); // 유효하지 않은 인덱스면 빈 슬롯 반환
}

bool UFactoryInventoryComponent::RequestTransferItem(UFactoryInventoryComponent* SourceInventory, int32 SourceSlotIndex, 
                                                     int32 TargetSlotIndex)
{
	if (!SourceInventory || !SourceInventory->InventorySlots.IsValidIndex(SourceSlotIndex)) return false;
	if (SourceInventory == this && SourceSlotIndex == TargetSlotIndex) return false; // 같은 슬롯이면 무시
	
	const FFactorySlot& SourceSlot = SourceInventory->InventorySlots[SourceSlotIndex];
	if (SourceSlot.IsEmpty()) return false;	// 옮길 아이템이 없는 경우
	
	const UFactoryItemData* ItemInSource = SourceSlot.ItemData;
	const int32 AmountInSource = FMath::Min(SourceSlot.Amount, FFactorySlot::MaxCapacity);
	
	if (TargetSlotIndex < 0)	// 슬롯 미지정. 자동 추가
	{
		int32 AddedAmount = AutoAddItem(ItemInSource, AmountInSource);
		if (AddedAmount > 0)
		{
			if (!SourceInventory->RemoveItemFromTargetSlot(SourceSlotIndex, AddedAmount))
			{
				AutoRemoveItem(ItemInSource, AddedAmount); // 롤백
				return false;
			}
			return true;
		}
		return false;
	}
	
	// 슬롯 지정. 해당 슬롯에 넣을 수 있는지 체크
	if (!InventorySlots.IsValidIndex(TargetSlotIndex)) return false;
	FFactorySlot& TargetSlot = InventorySlots[TargetSlotIndex];
	if (TargetSlot.IsEmpty() || TargetSlot.ItemData == ItemInSource)	// 슬롯이 비어있거나 같은 아이템이 있는 경우
	{
		int32 AvailableSpace = TargetSlot.GetAvailableSpace();
		if (AvailableSpace <= 0) return false; // 슬롯이 이미 가득 찬 경우
		
		int32 AmountToTransfer = FMath::Min(AmountInSource, AvailableSpace);
		
		if (!SourceInventory->RemoveItemFromTargetSlot(SourceSlotIndex, AmountToTransfer))
		{
			return false; // 원본에서 제거 실패
		}
		
		int32 AddedAmount = AddItemToTargetSlot(TargetSlotIndex, ItemInSource, AmountToTransfer);
		if (AddedAmount < AmountToTransfer)
		{
			// 추가 실패한 만큼 원본으로 롤백
			SourceInventory->AddItemToTargetSlot(SourceSlotIndex, ItemInSource, AmountToTransfer - AddedAmount);
			return false;
		}
		
		return true;
	}
	else
	{
		// 다른 아이템이 있는 경우, 서로 아이템을 교환하는 로직
		if (SourceInventory != this) return false; //	같은 인벤토리에서만 허용
		FFactorySlot TempSlot = TargetSlot;
        
		TargetSlot = SourceSlot;
		SourceInventory->InventorySlots[SourceSlotIndex] = TempSlot;

		// 양쪽 UI 갱신
		OnSlotUpdated.Broadcast(TargetSlotIndex, TargetSlot);
		SourceInventory->OnSlotUpdated.Broadcast(SourceSlotIndex, SourceInventory->InventorySlots[SourceSlotIndex]);
        
		return true;
	}
}

int32 UFactoryInventoryComponent::AddItemToTargetSlot(int32 SlotIndex, const UFactoryItemData* ItemData, int32 Amount)
{
	if (!InventorySlots.IsValidIndex(SlotIndex) || Amount <= 0 || ItemData == nullptr) return 0;
	
	FFactorySlot& TargetSlot = InventorySlots[SlotIndex];
	
	if (TargetSlot.IsEmpty())
	{
		TargetSlot.ItemData = ItemData;
		int32 ClampedAmount = FMath::Clamp(Amount, 0, FFactorySlot::MaxCapacity);
		TargetSlot.Amount = ClampedAmount;
		OnSlotUpdated.Broadcast(SlotIndex, TargetSlot);
		return ClampedAmount;
	}
	
	if (TargetSlot.ItemData == ItemData)
	{
		int32 AvailableSpace = TargetSlot.GetAvailableSpace();
		if (AvailableSpace <= 0) return 0; // 슬롯이 이미 가득 찬 경우
		
		int32 AmountToAdd = FMath::Min(Amount, AvailableSpace);
		TargetSlot.Amount += AmountToAdd;
		OnSlotUpdated.Broadcast(SlotIndex, TargetSlot);
		return AmountToAdd;
	}
	
	return 0;
}

int32 UFactoryInventoryComponent::AutoAddItem(const UFactoryItemData* ItemData, int32 Amount)
{
	if (Amount <= 0 || ItemData == nullptr) return 0;
    
	int32 RemainingAmount = Amount;

	// 기존에 같은 아이템이 있는 덜 찬 슬롯을 찾아 채워넣기
	for (int i = 0; i < InventorySlots.Num(); i++)
	{
		if (InventorySlots[i].ItemData == ItemData && !InventorySlots[i].IsFull())
		{
			int32 Added = AddItemToTargetSlot(i, ItemData, RemainingAmount);
			RemainingAmount -= Added;
			if (RemainingAmount <= 0) return Amount; // 다 넣었으면 종료
		}
	}
    
	// 그래도 남았다면 완전 빈 슬롯을 찾아 넣기
	for (int i = 0; i < InventorySlots.Num(); i++)
	{
		if (InventorySlots[i].IsEmpty())
		{
			int32 Added = AddItemToTargetSlot(i, ItemData, RemainingAmount);
			RemainingAmount -= Added;
			if (RemainingAmount <= 0) return Amount;
		}
	}
    
	return Amount - RemainingAmount; // 최종적으로 인벤토리에 들어간 총 개수 반환
}

bool UFactoryInventoryComponent::RemoveItemFromTargetSlot(int32 SlotIndex, int32 Amount)
{
	if (!InventorySlots.IsValidIndex(SlotIndex) || Amount <= 0) return false;
	
	FFactorySlot& TargetSlot = InventorySlots[SlotIndex];
	if (TargetSlot.IsEmpty()) return false;
	if (TargetSlot.Amount < Amount) return false;
	
	TargetSlot.Amount -= Amount;
	TargetSlot.Amount = FMath::Clamp(TargetSlot.Amount, 0, FFactorySlot::MaxCapacity);
	if (TargetSlot.Amount == 0)
	{
		TargetSlot.Clear();
	}
	
	OnSlotUpdated.Broadcast(SlotIndex, TargetSlot);
	return true;
}

bool UFactoryInventoryComponent::AutoRemoveItem(const UFactoryItemData* ItemData, int32 Amount)
{
	if (Amount <= 0 || ItemData == nullptr) return false;
	
	if (GetTotalItemAmount(ItemData) < Amount) return false;
	
	int32 RemainingAmount = Amount;
	
	for (int i = InventorySlots.Num() - 1; i >= 0; i--)
	{
		if (InventorySlots[i].ItemData == ItemData && !InventorySlots[i].IsEmpty())
		{
			int32 AmountToRemove = FMath::Min(RemainingAmount, InventorySlots[i].Amount);
			RemoveItemFromTargetSlot(i, AmountToRemove);
			RemainingAmount -= AmountToRemove;
			if (RemainingAmount <= 0) break; // 다 제거했으면 종료
		}
	}
	
	return true;
}

int32 UFactoryInventoryComponent::GetTotalItemAmount(const UFactoryItemData* ItemData) const
{
	int32 TotalItemAmount = 0;
	
	for (const FFactorySlot& Slot : InventorySlots)
	{
		if (Slot.ItemData == ItemData)
		{
			TotalItemAmount += Slot.Amount;
		}
	}
	
	return TotalItemAmount;
}

void UFactoryInventoryComponent::SortInventory()
{
	TMap<const UFactoryItemData*, int32> ItemCountMap;
    
	for (int i = 0; i < InventorySlots.Num(); i++)
	{
		if (InventorySlots[i].IsEmpty()) continue;
       
		// FindOrAdd의 기본값을 0으로 두고 더해야 기존 버그 방지
		ItemCountMap.FindOrAdd(InventorySlots[i].ItemData, 0) += InventorySlots[i].Amount;
		InventorySlots[i].Clear();
	}
    
	int32 SlotIndex = 0;
	for (const auto& Pair : ItemCountMap)
	{
		const UFactoryItemData* ItemData = Pair.Key;
		int32 TotalAmount = Pair.Value;
       
		while (TotalAmount > 0 && SlotIndex < InventorySlots.Num())
		{
			int32 AmountToAdd = FMath::Min(TotalAmount, FFactorySlot::MaxCapacity);
			InventorySlots[SlotIndex].ItemData = ItemData;
			InventorySlots[SlotIndex].Amount = AmountToAdd;
            
			TotalAmount -= AmountToAdd;
			SlotIndex++;
		}
		if (SlotIndex >= InventorySlots.Num()) break;
	}
    
	// 정렬 후 모든 슬롯들의 UI 갱신
	for (int i = 0; i < InventorySlots.Num(); i++)
	{
		OnSlotUpdated.Broadcast(i, InventorySlots[i]);
	}
}




