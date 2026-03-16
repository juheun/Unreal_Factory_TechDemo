// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Warehouse/FactoryWarehouseSlotWidget.h"

#include "Components/Image.h"
#include "Player/Component/FactoryInventoryComponent.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"
#include "UI/Core/FactoryItemDragDropOperation.h"
#include "UI/Inventory/FactoryInventorySlotWidget.h"


void UFactoryWarehouseSlotWidget::InitWarehouseSlot(const UFactoryItemData* ItemData, int32 Amount)
{
	SlotItemData = ItemData;
	SlotAmount = Amount;
	
	UpdateSlotVisual(SlotItemData, SlotAmount);
}

void UFactoryWarehouseSlotWidget::UpdateSlotVisual(const UFactoryItemData* ItemData, int32 Amount)
{
	Super::UpdateSlotVisual(ItemData, Amount);
	
	if (ItemData != nullptr && Amount == 0)
	{
		if (ItemIcon) ItemIcon->SetRenderOpacity(0.5f);
	}
}

bool UFactoryWarehouseSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	UFactoryItemDragDropOperation* DragOperation = Cast<UFactoryItemDragDropOperation>(InOperation);
	if (!DragOperation) return false;
	if (DragOperation->SourceSlotWidget == this) return false;	// 자기 자신에게 드롭한 경우 무시
	
	const UFactoryItemData* ItemData = DragOperation->ItemData;
	int32 DraggedAmount = DragOperation->DraggedAmount;
	
	UFactoryWarehouseSubsystem* Warehouse = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
	if (!Warehouse) return false;
	
	// 출발지가 인벤토리 슬롯인 경우
	if (UFactoryInventorySlotWidget* SourceInventorySlotWidget = Cast<UFactoryInventorySlotWidget>(DragOperation->SourceSlotWidget))
	{
		if (UFactoryInventoryComponent* SourceInventory = SourceInventorySlotWidget->GetInventoryComponent())
		{
			int32 CurrentWarehouseAmount = Warehouse->GetItemAmount(ItemData);
			int32 SpaceLeft = FMath::Max(0, Warehouse->GetMaxItemAmount() - CurrentWarehouseAmount);
			
			int32 AmountToTransfer = FMath::Min(DraggedAmount, SpaceLeft);
			if (AmountToTransfer > 0)
			{
				if (SourceInventory->RemoveItemFromTargetSlot(SourceInventorySlotWidget->GetSlotIndex(), AmountToTransfer))
				{
					// 만약 인벤토리에서 창고에 추가된만큼 아이템 제거에 실패한 경우
					Warehouse->AddItem(ItemData, AmountToTransfer);
					return true;
				}
			}
		}
	}
	
	return false;
}
