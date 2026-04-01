// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerContext/FactoryPlayerContextWidget.h"

#include "Components/TextBlock.h"
#include "Player/Input/FactoryPlayerController.h"

void UFactoryPlayerContextWidget::UpdateViewModeUI(const FString& ViewModeString) const
{
	if (!ViewModeText) return;
	
	ViewModeText->SetText(FText::FromString(ViewModeString));
}

void UFactoryPlayerContextWidget::UpdatePlacementUI(const FString& PlacementModeString) const
{
	if (!PlacementModeText) return;
	
	PlacementModeText->SetText(FText::FromString(PlacementModeString));
}

void UFactoryPlayerContextWidget::UpdateHotkeyHints(const FString& HotKeyHintString) const
{
	if (!HotkeyHintText) return;
	
	if (HotKeyHintString.IsEmpty())
	{
		HotkeyHintText->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		HotkeyHintText->SetVisibility(ESlateVisibility::HitTestInvisible);
		HotkeyHintText->SetText(FText::FromString(HotKeyHintString));
	}
}
