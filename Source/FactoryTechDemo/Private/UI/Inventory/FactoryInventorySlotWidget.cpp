// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/FactoryInventorySlotWidget.h"

#include "Logistics/Machines/FactoryMachineBase.h"
#include "Player/Component/FactoryInventoryComponent.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"
#include "UI/Core/FactoryItemDragDropOperation.h"
#include "UI/Facility/FactoryFacilitySlotWidget.h"
#include "UI/Warehouse/FactoryWarehouseSlotWidget.h"

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
	
	const UFactoryItemData* DraggedItemData = DragOperation->ItemData;
	int32 DraggedAmount = DragOperation->DraggedAmount;
	
	UFactoryInventoryComponent* TargetInventory = LinkedInventory.Get();
	if (!TargetInventory) return false;
	
	// 출발지가 인벤토리 슬롯인 경우 (같은 인벤토리 내 이동 혹은 다른 인벤토리에서 이동)
	if (UFactoryInventorySlotWidget* SourceInventorySlotWidget = Cast<UFactoryInventorySlotWidget>(DragOperation->SourceSlotWidget))
	{
		if (UFactoryInventoryComponent* SourceInventory = SourceInventorySlotWidget->GetInventoryComponent())
		{
			return TargetInventory->RequestTransferItem(SourceInventory, SourceInventorySlotWidget->GetSlotIndex(), SlotIndex);
		}
	}
	else if (UFactoryWarehouseSlotWidget* SourceWarehouseSlotWidget = Cast<UFactoryWarehouseSlotWidget>(DragOperation->SourceSlotWidget))
	{	
		// 출발지가 창고인경우
		UFactoryWarehouseSubsystem* Warehouse = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
		if (!Warehouse) return false;
		
		FFactorySlot TargetSlot = TargetInventory->GetSlotData(SlotIndex);
		if (TargetSlot.IsEmpty() || TargetSlot.ItemData == DraggedItemData)	// 비어있거나 같은 종류의 아이템일때만 전송 수행
		{
			int32 AvailableSpace = TargetSlot.GetAvailableSpace();
			int32 AmountToTransfer = FMath::Min(AvailableSpace, DraggedAmount);
			if (AmountToTransfer > 0)
			{
				if (Warehouse->TryRemoveItem(DraggedItemData, AmountToTransfer))
				{
					TargetInventory->AddItemToTargetSlot(SlotIndex, DraggedItemData, AmountToTransfer);
					return true;
				}
			}
		}
	}
	else if (UFactoryFacilitySlotWidget* SourceFacilitySlotWidget = Cast<UFactoryFacilitySlotWidget>(DragOperation->SourceSlotWidget))
	{
		FFactorySlot TakenSlot;
		
		if (AFactoryMachineBase* Machine = Cast<AFactoryMachineBase>(SourceFacilitySlotWidget->OwnerFacility.Get()))
		{
			if (Machine->TryTakeItemFromBuffer(SourceFacilitySlotWidget->bIsInputSlot, SourceFacilitySlotWidget->FacilitySlotIndex, DraggedAmount, TakenSlot))
			{
				TargetInventory->AddItemToTargetSlot(SlotIndex, TakenSlot.ItemData, TakenSlot.Amount);
				return true;
			}
		}
	}
	
	return false;
}