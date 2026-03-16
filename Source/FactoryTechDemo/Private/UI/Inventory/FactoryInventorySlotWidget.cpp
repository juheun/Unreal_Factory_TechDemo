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
	if (DragOperation->SourceSlotWidget == this) return false;	// 자기 자신에게 드롭한 경우 무시
	
	UFactoryInventoryComponent* TargetInventory = LinkedInventory.Get();
	if (!TargetInventory) return false;
	
	// 출발지가 인벤토리 슬롯인 경우 (같은 인벤토리 내 이동 혹은 다른 인벤토리에서 이동)
	if (UFactoryInventorySlotWidget* SourceSlot = Cast<UFactoryInventorySlotWidget>(DragOperation->SourceSlotWidget))
	{
		if (UFactoryInventoryComponent* SourceInventory = SourceSlot->LinkedInventory.Get())
		{
			return TargetInventory->RequestTransferItem(SourceInventory, SourceSlot->SlotIndex, SlotIndex);
		}
	}
	
	// TODO : 출발지가 창고 슬롯인 경우
	return false;
}