// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Core/FactoryItemSelectButton.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Items/FactoryItemData.h"

void UFactoryItemSelectButton::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ItemButton)
	{
		ItemButton->OnClicked.RemoveDynamic(this, &UFactoryItemSelectButton::HandleButtonClicked);
		ItemButton->OnClicked.AddDynamic(this, &UFactoryItemSelectButton::HandleButtonClicked);
	}
}

void UFactoryItemSelectButton::InitButton(UFactoryItemData* InItemData)
{
	CachedItemData = InItemData;
	if (CachedItemData)
	{
		if (ItemIcon && CachedItemData->ItemIcon)
		{
			ItemIcon->SetBrushFromTexture(CachedItemData->ItemIcon);
		}
		if (ItemNameText)
		{
			ItemNameText->SetText(CachedItemData->ItemName);
		}
	}
}

void UFactoryItemSelectButton::HandleButtonClicked()
{
	if (CachedItemData)
	{
		OnButtonClicked.Broadcast(CachedItemData);
	}
}
