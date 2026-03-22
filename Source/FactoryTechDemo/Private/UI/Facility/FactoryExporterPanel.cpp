// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Facility/FactoryExporterPanel.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Logistics/FactoryWarehouseExporter.h"
#include "Subsystems/FactoryDataSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"
#include "UI/Core/FactoryBaseSlotWidget.h"
#include "UI/Facility/FactoryItemSelectionPopup.h"

void UFactoryExporterPanel::InitPanel(AFactoryPlaceObjectBase* PlaceObject)
{
	Super::InitPanel(PlaceObject);
	
	// 중복 구독 방지
	if (AFactoryWarehouseExporter* OldExporter = CachedExporter.Get())
	{
		OldExporter->OnTargetItemChanged.RemoveDynamic(this, &UFactoryExporterPanel::OnExporterItemChanged);
		OldExporter->OnWarehouseAmountUpdated.RemoveDynamic(this, &UFactoryExporterPanel::OnWarehouseItemAmountUpdated);
	}
	
	CachedExporter = Cast<AFactoryWarehouseExporter>(PlaceObject);
	
	if (AFactoryWarehouseExporter* Exporter = CachedExporter.Get())
	{
		Exporter->OnTargetItemChanged.AddDynamic(this, &UFactoryExporterPanel::OnExporterItemChanged);
		Exporter->OnWarehouseAmountUpdated.AddDynamic(this, &UFactoryExporterPanel::OnWarehouseItemAmountUpdated);
		
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
		
		// TargetItem이 있는경우 패널 처음 열때에 한해 갯수 초기화
		if (Exporter->GetTargetItem())
		{
			if (UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>())
			{
				int32 InitialAmount = WarehouseSubsystem->GetItemAmount(Exporter->GetTargetItem());
				OnWarehouseItemAmountUpdated(InitialAmount);
			}
		}
		else
		{
			OnWarehouseItemAmountUpdated(0);
		}
	}
}

void UFactoryExporterPanel::NativeDestruct()
{
	if (AFactoryWarehouseExporter* Exporter = CachedExporter.Get())
	{
		Exporter->OnTargetItemChanged.RemoveDynamic(this, &UFactoryExporterPanel::OnExporterItemChanged);
		Exporter->OnWarehouseAmountUpdated.RemoveDynamic(this, &UFactoryExporterPanel::OnWarehouseItemAmountUpdated);

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
			ItemSelectionPopup->OpenPopup(ItemsToShow);
		}
	}
}

void UFactoryExporterPanel::OnPopupItemSelected(const UFactoryItemData* SelectedItem)
{
	if (AFactoryWarehouseExporter* Exporter = CachedExporter.Get())
	{
		Exporter->SetTargetItem(SelectedItem);
	}
}

void UFactoryExporterPanel::OnExporterItemChanged(const UFactoryItemData* NewItem)
{
	if (TargetItemSlot)
	{
		TargetItemSlot->UpdateSlotVisual(NewItem, 0);
	}
}

void UFactoryExporterPanel::OnWarehouseItemAmountUpdated(const int32 CurrentAmount)
{
	if (WarehouseAmountText)
	{
		WarehouseAmountText->SetText(FText::Format(FText::FromString(TEXT("{0}")), FText::AsNumber(CurrentAmount)));
	}
}
