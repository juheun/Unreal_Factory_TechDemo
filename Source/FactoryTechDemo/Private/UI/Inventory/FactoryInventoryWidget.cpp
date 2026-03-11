// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/FactoryInventoryWidget.h"

#include "Components/UniformGridPanel.h"
#include "Player/Component/FactoryInventoryComponent.h"
#include "UI/Inventory/FactoryInventorySlotWidget.h"

void UFactoryInventoryWidget::InitInventory(UFactoryInventoryComponent* InventoryComponent, int32 Columns)
{
	if (!InventoryComponent || !SlotWidgetBP || !InventoryGridPanel) return;
	
	InventoryGridPanel->ClearChildren();
	SlotWidgets.Empty();
	
	int32 MaxSlots = InventoryComponent->GetMaxItemSlotCount();
	for (int i = 0; i < MaxSlots; i++)
	{
		if (UFactoryInventorySlotWidget* NewSlotWidget = CreateWidget<UFactoryInventorySlotWidget>(this, SlotWidgetBP))
		{
			NewSlotWidget->InitInventorySlot(InventoryComponent, i);
			int32 Row = i / Columns;
			int32 Column = i % Columns;
			InventoryGridPanel->AddChildToUniformGrid(NewSlotWidget, Row, Column);
			SlotWidgets.Add(NewSlotWidget);
		}
	}
}
