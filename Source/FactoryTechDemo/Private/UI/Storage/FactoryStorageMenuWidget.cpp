// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Storage/FactoryStorageMenuWidget.h"

#include "UI/Inventory/FactoryInventoryWidget.h"
#include "UI/Warehouse/FactoryWarehousePanelWidget.h"

void UFactoryStorageMenuWidget::OpenMenu(UFactoryInventoryComponent* PlayerInventory, EFactoryMenuMode MenuMode)
{
	if (WarehousePanel)
	{
		if (MenuMode == EFactoryMenuMode::Warehouse)
		{
			WarehousePanel->SetVisibility(ESlateVisibility::Visible);
			WarehousePanel->InitPanel();
		}
		else
		{
			WarehousePanel->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 인벤토리 패널 (항상 보임)
	if (InventoryPanel && PlayerInventory)
	{
		InventoryPanel->SetVisibility(ESlateVisibility::Visible);
		InventoryPanel->InitInventory(PlayerInventory); 
	}
}
