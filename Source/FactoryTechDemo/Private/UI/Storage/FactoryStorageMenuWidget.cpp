// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Storage/FactoryStorageMenuWidget.h"

#include "Components/Border.h"
#include "Placement/FactoryObjectData.h"
#include "Placement/FactoryPlaceObjectBase.h"
#include "UI/Facility/FactoryFacilityPanelBase.h"
#include "UI/Inventory/FactoryInventoryWidget.h"
#include "UI/Warehouse/FactoryWarehousePanelWidget.h"

void UFactoryStorageMenuWidget::OpenMenu(UFactoryInventoryComponent* PlayerInventory, EFactoryMenuMode MenuMode, 
	AFactoryPlaceObjectBase* TargetFacility)
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
	
	if (FacilityPanelContainer)
	{
		if ((MenuMode == EFactoryMenuMode::Facility || MenuMode == EFactoryMenuMode::FacilityOnly) && 
		   TargetFacility && TargetFacility->GetObjectData() && TargetFacility->GetObjectData()->FacilityPanelBP)
		{
			FacilityPanelContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
          
			TSubclassOf<UFactoryFacilityPanelBase> FacilityPanelBP = TargetFacility->GetObjectData()->FacilityPanelBP;
			UFactoryFacilityPanelBase* PanelWidget = nullptr;
          
			if (CachedFacilityPanels.Contains(FacilityPanelBP))
			{
				PanelWidget = CachedFacilityPanels[FacilityPanelBP];
			}
			else
			{
				PanelWidget = CreateWidget<UFactoryFacilityPanelBase>(this, FacilityPanelBP);
				if (PanelWidget) CachedFacilityPanels.Add(FacilityPanelBP, PanelWidget);
			}
          
			if (PanelWidget)
			{
				FacilityPanelContainer->ClearChildren();
				FacilityPanelContainer->AddChild(PanelWidget);
				PanelWidget->InitPanel(TargetFacility);
			}
		}
		else
		{
			FacilityPanelContainer->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 인벤토리 패널
	if (InventoryPanel && PlayerInventory)
	{
		if (MenuMode == EFactoryMenuMode::FacilityOnly)
		{
			InventoryPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			InventoryPanel->SetVisibility(ESlateVisibility::Visible);
			InventoryPanel->InitInventory(PlayerInventory); 
		}
	}
}
