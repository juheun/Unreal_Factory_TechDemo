// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/FactorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Player/Component/FactoryInventoryComponent.h"
#include "UI/Core/FactoryItemDragDropOperation.h"
#include "UI/Core/FFactorySlot.h"
#include "Items/FactoryItemData.h"

void UFactorySlotWidget::InitSlot(UFactoryInventoryComponent* InventoryComponent, EFactorySlotType Type, int32 Index)
{
	if (LinkedInventory.IsValid())
	{
		LinkedInventory->OnSlotUpdated.RemoveAll(this);
	}
	
	LinkedInventory = InventoryComponent;
	SlotType = Type;
	SlotIndex = Index;
	
	if (UFactoryInventoryComponent* Inventory = LinkedInventory.Get())
	{
		Inventory->OnSlotUpdated.AddDynamic(this, &UFactorySlotWidget::OnSlotDataChanged);
	}
}

void UFactorySlotWidget::OnSlotDataChanged(int32 UpdatedSlotIndex, FFactorySlot UpdatedSlotData)
{
	if (SlotIndex == UpdatedSlotIndex)
	{
		UpdateSlotInfo(UpdatedSlotData.ItemData, UpdatedSlotData.Amount);
	}
}

void UFactorySlotWidget::UpdateSlotInfo(const UFactoryItemData* ItemData, int32 Amount)
{
	CurrentItemData = ItemData;
	CurrentAmount = Amount;
	
	if (CurrentItemData == nullptr || CurrentAmount <= 0)
	{
		if (ItemIcon) ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		if (AmountText) AmountText->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		if (ItemIcon)
		{
			if (CurrentItemData->ItemICon)
			{
				ItemIcon->SetBrushFromTexture(CurrentItemData->ItemICon);
			}
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
		}
		if (AmountText)
		{
			AmountText->SetText(FText::AsNumber(CurrentAmount));
			AmountText->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

FReply UFactorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && CurrentItemData != nullptr)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(
			InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UFactorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	UFactoryItemDragDropOperation* DragOperation = Cast<UFactoryItemDragDropOperation>(
		UWidgetBlueprintLibrary::CreateDragDropOperation(UFactoryItemDragDropOperation::StaticClass()));
	
	if (DragOperation)
	{
		DragOperation->SourceType = SlotType;
		DragOperation->SourceSlotWidget = this;
		DragOperation->ItemData = CurrentItemData;
		DragOperation->DraggedAmount = CurrentAmount;
		
		// 마우스 따라다닐 UI
		if (UFactorySlotWidget* VisualWidget = CreateWidget<UFactorySlotWidget>(GetWorld(), GetClass()))
		{
			VisualWidget->UpdateSlotInfo(CurrentItemData, CurrentAmount);
			DragOperation->DefaultDragVisual = VisualWidget;
		}
        
		DragOperation->Pivot = EDragPivot::MouseDown;
		OutOperation = DragOperation;
	}
}

bool UFactorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	UFactoryItemDragDropOperation* DragOperation = Cast<UFactoryItemDragDropOperation>(InOperation);
	if (!DragOperation) return false;
	if (DragOperation->SourceSlotWidget == this) return false;	// 같은 슬롯에서 드래그 시작한 경우 무시
	
	UFactoryInventoryComponent* TargetInventory = LinkedInventory.Get();	// 드롭된 슬롯의 인벤토리
	UFactoryInventoryComponent* SourceInventory = 
		DragOperation->SourceSlotWidget->LinkedInventory.Get();	// 드래그 시작한 슬롯의 인벤토리
	
	if (!TargetInventory || !SourceInventory) return false;
	
	return TargetInventory->RequestTransferItem(SourceInventory, DragOperation->SourceSlotWidget->SlotIndex, SlotIndex);
}
