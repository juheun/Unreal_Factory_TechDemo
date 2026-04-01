// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Interaction//FactoryInteractionWidget.h"

#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Interfaces/FactoryInteractable.h"


void UFactoryInteractionWidget::SetInteractionTextList(const TArray<FInteractionOption>& Options, int32 SelectedIndex)
{
	if (!InteractionListPanel) return;
	
	InteractionListPanel->ClearChildren();
	
	for (int32 i = 0; i < Options.Num(); i++)
	{
		UTextBlock* OptionText = NewObject<UTextBlock>(this);
		if (!OptionText) continue;
		
		// 선택 여부에 따른 색상 설정 및 텍스트 내용 설정
		if (i == SelectedIndex)
		{
			FString SelectedStr = FString::Printf(TEXT("▶ %s"), *Options[i].DisplayText.ToString());
			OptionText->SetText(FText::FromString(SelectedStr));
			OptionText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.0f, 1.0f)));
		}
		else
		{
			FString UnselectedStr = FString::Printf(TEXT("    %s"), *Options[i].DisplayText.ToString());
			OptionText->SetText(FText::FromString(UnselectedStr));
			OptionText->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 0.4f)));
		}
		
		if (UVerticalBoxSlot* BoxSlot = InteractionListPanel->AddChildToVerticalBox(OptionText))
		{
			BoxSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 4.f));
		}
	}
}
