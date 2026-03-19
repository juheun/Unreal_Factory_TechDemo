// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Facility/FactoryExporterPanel.h"

#include "Components/Button.h"
#include "Logistics/FactoryWarehouseExporter.h"
#include "Subsystems/FactoryDataSubsystem.h"
#include "UI/Core/FactoryBaseSlotWidget.h"
#include "UI/Facility/FactoryItemSelectionPopup.h"

void UFactoryExporterPanel::InitPanel(AFactoryPlaceObjectBase* PlaceObject)
{
	Super::InitPanel(PlaceObject);
	
	if (AFactoryWarehouseExporter* OldExporter = CachedExporter.Get())
	{
		OldExporter->OnTargetItemChanged.RemoveDynamic(this, &UFactoryExporterPanel::OnExporterItemChanged);
	}
	
	CachedExporter = Cast<AFactoryWarehouseExporter>(PlaceObject);
	
	if (AFactoryWarehouseExporter* Exporter = CachedExporter.Get())
	{
		// 중복 구독 방지
		Exporter->OnTargetItemChanged.AddDynamic(this, &UFactoryExporterPanel::OnExporterItemChanged);
		
		if (SelectPanelOpenButton)
		{
			SelectPanelOpenButton->OnClicked.RemoveDynamic(this, &UFactoryExporterPanel::OnSelectPanelOpenButtonClicked);
			SelectPanelOpenButton->OnClicked.AddDynamic(this, &UFactoryExporterPanel::OnSelectPanelOpenButtonClicked);
		}
		
		if (ItemSelectionPopup)
		{
			ItemSelectionPopup->SetVisibility(ESlateVisibility::Collapsed);
			
			ItemSelectionPopup->OnItemSelected.RemoveDynamic(this, &UFactoryExporterPanel::OnPopupItemSelected);
			ItemSelectionPopup->OnItemSelected.AddDynamic(this, &UFactoryExporterPanel::OnPopupItemSelected);
		}

		if (TargetItemSlot)
		{
			// 선택 아이템 확인용이기에 상호작용 차단
			TargetItemSlot->SetInteractable(false);
			
			TargetItemSlot->UpdateSlotVisual(Exporter->GetTargetItem(), 0); 
		}
	}
}

void UFactoryExporterPanel::NativeDestruct()
{
	if (AFactoryWarehouseExporter* Exporter = CachedExporter.Get())
	{
		Exporter->OnTargetItemChanged.RemoveDynamic(this, &UFactoryExporterPanel::OnExporterItemChanged);
	}
	
	Super::NativeDestruct();
}

void UFactoryExporterPanel::OnSelectPanelOpenButtonClicked()
{
	if (UFactoryDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UFactoryDataSubsystem>())
	{
		TArray<UFactoryItemData*> ItemsToShow;
		
		ItemsToShow.Append(DataSubsystem->GetItemDatasByCategory(EFactoryItemCategory::Resource));
		ItemsToShow.Append(DataSubsystem->GetItemDatasByCategory(EFactoryItemCategory::Consumable));
        
		if (ItemSelectionPopup)
		{
			ItemSelectionPopup->OpenPopup(ItemsToShow); // 팝업으로 슝!
		}
	}
}

void UFactoryExporterPanel::OnPopupItemSelected(UFactoryItemData* SelectedItem)
{
	if (AFactoryWarehouseExporter* Exporter = CachedExporter.Get())
	{
		Exporter->SetTargetItem(SelectedItem);
	}
}

void UFactoryExporterPanel::OnExporterItemChanged(UFactoryItemData* NewItem)
{
	if (TargetItemSlot)
	{
		TargetItemSlot->UpdateSlotVisual(NewItem, 0);
	}
}
