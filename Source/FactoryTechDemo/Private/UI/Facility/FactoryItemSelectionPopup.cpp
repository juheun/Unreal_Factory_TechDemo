// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Facility/FactoryItemSelectionPopup.h"

#include "Components/Button.h"
#include "Components/WrapBox.h"
#include "UI/Core/FactoryItemSelectButton.h"

void UFactoryItemSelectionPopup::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UFactoryItemSelectionPopup::ClosePopup);
		CloseButton->OnClicked.AddDynamic(this, &UFactoryItemSelectionPopup::ClosePopup);
	}
}

void UFactoryItemSelectionPopup::OpenPopup(const TArray<UFactoryItemData*>& ItemDatas)
{
	if (!ItemContainer || !ItemButtonBP) return;
	
	ItemContainer->ClearChildren();
	
	for (UFactoryItemData* ItemData : ItemDatas)
	{
		if (UFactoryItemSelectButton* ItemButton = CreateWidget<UFactoryItemSelectButton>(this, ItemButtonBP))
		{
			ItemButton->InitButton(ItemData);
			ItemButton->OnButtonClicked.RemoveDynamic(this, &UFactoryItemSelectionPopup::HandleItemButtonClicked);
			ItemButton->OnButtonClicked.AddDynamic(this, &UFactoryItemSelectionPopup::HandleItemButtonClicked);
			ItemContainer->AddChildToWrapBox(ItemButton);
		}
	}
	
	SetVisibility(ESlateVisibility::Visible);
}

void UFactoryItemSelectionPopup::ClosePopup()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UFactoryItemSelectionPopup::HandleItemButtonClicked(UFactoryItemData* SelectedItem)
{
	OnItemSelected.Broadcast(SelectedItem);
	ClosePopup();
}
