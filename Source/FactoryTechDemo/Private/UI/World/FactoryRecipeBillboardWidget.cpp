// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/FactoryRecipeBillboardWidget.h"

#include "Components/Image.h"

void UFactoryRecipeBillboardWidget::SetRecipeIcon(UTexture2D* InIcon)
{
	if (RecipeIconImage && InIcon)
	{
		RecipeIconImage->SetBrushFromTexture(InIcon);
		RecipeIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else if (RecipeIconImage)
	{
		// 아이콘이 없으면 숨김 처리 (예: 제작 중이 아닐 때)
		RecipeIconImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}
