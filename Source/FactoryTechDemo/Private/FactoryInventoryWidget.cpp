// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryInventoryWidget.h"

#include "Components/UniformGridPanel.h"
#include "Inventory/FactoryInventoryComponent.h"
#include "Inventory/FactorySlotWidget.h"

void UFactoryInventoryWidget::InitInventory(UFactoryInventoryComponent* InventoryComponent, int32 Columns)
{
	if (!InventoryComponent || !SlotWidgetBP || !InventoryGridPanel) return;
	
	InventoryGridPanel->ClearChildren();
	SlotWidgets.Empty();
	
	int32 MaxSlots = InventoryComponent->GetMaxItemSlotCount();
	for (int i = 0; i < MaxSlots; i++)
	{
		if (UFactorySlotWidget* NewSlotWidget = CreateWidget<UFactorySlotWidget>(this, SlotWidgetBP))
		{
			NewSlotWidget->InitSlot(InventoryComponent, EFactorySlotType::Inventory, i);
			int32 Row = i / Columns;
			int32 Column = i % Columns;
			InventoryGridPanel->AddChildToUniformGrid(NewSlotWidget, Row, Column);
			SlotWidgets.Add(NewSlotWidget);
		}
	}
}
