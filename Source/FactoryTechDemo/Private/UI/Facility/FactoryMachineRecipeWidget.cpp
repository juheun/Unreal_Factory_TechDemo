// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Facility/FactoryMachineRecipeWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Items/FactoryRecipeData.h"
#include "UI/Core/FactoryBaseSlotWidget.h"

void UFactoryMachineRecipeWidget::Init(const UFactoryRecipeData* RecipeData)
{
	if (!RecipeData) return;
	
	if (SlotWidgetClass && InputSlotsContainer)
	{
		InputSlotsContainer->ClearChildren();
		
		for (const FRecipeIngredient& Input : RecipeData->Inputs)
		{
			if (UFactoryBaseSlotWidget* InputSlot = CreateWidget<UFactoryBaseSlotWidget>(this, SlotWidgetClass))
			{
				InputSlot->SetInteractable(false);
				InputSlot->UpdateSlotVisual(Input.ItemData, Input.Amount);
				InputSlotsContainer->AddChildToHorizontalBox(InputSlot);
			}
		}
	}
	
	if (OutputSlot)
	{
		OutputSlot->SetInteractable(false);
		OutputSlot->UpdateSlotVisual(RecipeData->Output.ItemData, RecipeData->Output.Amount);
	}
	
	if (ProcessingTimeText)
	{
		ProcessingTimeText->SetText(FText::FromString(
			FString::Printf(TEXT("%ds"),RecipeData->ProcessingTime)));
	}
}
