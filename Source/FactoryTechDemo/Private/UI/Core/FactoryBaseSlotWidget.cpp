// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Core/FactoryBaseSlotWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/Core/FactoryItemDragDropOperation.h"
#include "Items/FactoryItemData.h"

void UFactoryBaseSlotWidget::UpdateSlotVisual(const UFactoryItemData* ItemData, int32 Amount)
{
	SlotItemData = ItemData;
	SlotAmount = Amount;
	
	if (SlotItemData == nullptr)
	{
		if (ItemIcon) ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		if (AmountText) AmountText->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		if (ItemIcon)
		{
			if (SlotItemData->ItemIcon)
			{
				ItemIcon->SetBrushFromTexture(SlotItemData->ItemIcon);
			}
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
			
			if (SlotAmount <= 0)
			{
				ItemIcon->SetRenderOpacity(0.5f);
			}
			else
			{
				ItemIcon->SetRenderOpacity(1.0f);
			}
		}
		if (AmountText)
		{
			AmountText->SetText(FText::AsNumber(SlotAmount));
			AmountText->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

FReply UFactoryBaseSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsInteractable && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && SlotItemData != nullptr && SlotAmount > 0)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UFactoryBaseSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	if (!bIsInteractable) return;
	
	UFactoryItemDragDropOperation* DragOperation = Cast<UFactoryItemDragDropOperation>(
		UWidgetBlueprintLibrary::CreateDragDropOperation(UFactoryItemDragDropOperation::StaticClass()));
	
	if (DragOperation)
	{
		DragOperation->SourceSlotWidget = this;
		DragOperation->ItemData = SlotItemData;
		DragOperation->DraggedAmount = SlotAmount;
		
		// 마우스 따라다닐 UI
		if (UFactoryBaseSlotWidget* VisualWidget = CreateWidget<UFactoryBaseSlotWidget>(GetWorld(), GetClass()))
		{
			VisualWidget->UpdateSlotVisual(SlotItemData, SlotAmount);
			DragOperation->DefaultDragVisual = VisualWidget;
		}
		
		DragOperation->Pivot = EDragPivot::MouseDown;
		OutOperation = DragOperation;
	}
}