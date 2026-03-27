// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Facility/FactoryRecipeListPopupWidget.h"

#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "UI/Facility/FactoryMachineRecipeWidget.h"

void UFactoryRecipeListPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (CloseRecipeListPanelBtn)
	{
		CloseRecipeListPanelBtn->OnClicked.AddDynamic(this, &UFactoryRecipeListPopupWidget::OnCloseRecipeListClicked);
	}
}

void UFactoryRecipeListPopupWidget::InitPopup(const TArray<UFactoryRecipeData*>& Recipes)
{
	if (RecipeListScrollBox && RecipeWidgetClass)
	{
		RecipeListScrollBox->ClearChildren();
        
		for (const UFactoryRecipeData* Recipe : Recipes)
		{
			if (UFactoryMachineRecipeWidget* RecipeWidget = CreateWidget<UFactoryMachineRecipeWidget>(this, RecipeWidgetClass))
			{
				RecipeWidget->Init(Recipe);
				RecipeListScrollBox->AddChild(RecipeWidget);
			}
		}
	}
}

void UFactoryRecipeListPopupWidget::OnCloseRecipeListClicked()
{
	RemoveFromParent();
}
