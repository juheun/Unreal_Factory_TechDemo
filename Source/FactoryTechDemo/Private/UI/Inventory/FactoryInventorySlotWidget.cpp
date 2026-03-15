// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/FactoryInventorySlotWidget.h"
#include "Player/Component/FactoryInventoryComponent.h"
#include "UI/Core/FactoryItemDragDropOperation.h"

void UFactoryInventorySlotWidget::InitInventorySlot(UFactoryInventoryComponent* InventoryComponent, int32 Index)
{
	if (LinkedInventory.IsValid())
	{
		LinkedInventory->OnSlotUpdated.RemoveDynamic(this, &UFactoryInventorySlotWidget::OnSlotDataChanged);
	}
	
	LinkedInventory = InventoryComponent;
	SlotIndex = Index;
	
	if (UFactoryInventoryComponent* Inventory = LinkedInventory.Get())
	{
		Inventory->OnSlotUpdated.AddDynamic(this, &UFactoryInventorySlotWidget::OnSlotDataChanged);
		
		// UI 생성 직후 데이터를 한번 불러와서 한번 비주얼을 동기화시킴.
		FFactorySlot CurrentSlotData = Inventory->GetSlotData(Index);
		UpdateSlotVisual(CurrentSlotData.ItemData, CurrentSlotData.Amount);
	}
}

void UFactoryInventorySlotWidget::OnSlotDataChanged(int32 UpdatedSlotIndex, FFactorySlot UpdatedSlotData)
{
	if (SlotIndex == UpdatedSlotIndex)
	{
		UpdateSlotVisual(UpdatedSlotData.ItemData, UpdatedSlotData.Amount);
	}
}

bool UFactoryInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	UFactoryItemDragDropOperation* DragOperation = Cast<UFactoryItemDragDropOperation>(InOperation);
	if (!DragOperation) return false;
	
	// 상대방이 인벤토리 슬롯인지 확인
	UFactoryInventorySlotWidget* SourceSlot = Cast<UFactoryInventorySlotWidget>(DragOperation->SourceSlotWidget);
	if (!SourceSlot || SourceSlot == this) return false;
	
	UFactoryInventoryComponent* TargetInventory = LinkedInventory.Get();
	UFactoryInventoryComponent* SourceInventory = SourceSlot->LinkedInventory.Get();
	
	if (!TargetInventory || !SourceInventory) return false;
	
	// 진짜 물리적인 아이템 자리 교체!
	return TargetInventory->RequestTransferItem(SourceInventory, SourceSlot->SlotIndex, SlotIndex);
}