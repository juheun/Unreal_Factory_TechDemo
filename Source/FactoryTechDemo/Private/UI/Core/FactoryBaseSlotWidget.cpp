// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Core/FactoryBaseSlotWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/Core/FactoryItemDragDropOperation.h"
#include "Items/FactoryItemData.h"

void UFactoryBaseSlotWidget::UpdateSlotVisual(const UFactoryItemData* ItemData, int32 Amount)
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
			if (CurrentItemData->ItemIcon)
			{
				ItemIcon->SetBrushFromTexture(CurrentItemData->ItemIcon);
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

FReply UFactoryBaseSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && CurrentItemData != nullptr)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UFactoryBaseSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	UFactoryItemDragDropOperation* DragOperation = Cast<UFactoryItemDragDropOperation>(
		UWidgetBlueprintLibrary::CreateDragDropOperation(UFactoryItemDragDropOperation::StaticClass()));
	
	if (DragOperation)
	{
		DragOperation->SourceSlotWidget = this;
		DragOperation->ItemData = CurrentItemData;
		DragOperation->DraggedAmount = CurrentAmount;
		
		// 마우스 따라다닐 UI
		if (UFactoryBaseSlotWidget* VisualWidget = CreateWidget<UFactoryBaseSlotWidget>(GetWorld(), GetClass()))
		{
			VisualWidget->UpdateSlotVisual(CurrentItemData, CurrentAmount);
			DragOperation->DefaultDragVisual = VisualWidget;
		}
		
		DragOperation->Pivot = EDragPivot::MouseDown;
		OutOperation = DragOperation;
	}
}