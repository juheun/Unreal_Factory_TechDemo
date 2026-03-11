// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Interaction//FactoryInteractionWidget.h"

#include "Components/TextBlock.h"

void UFactoryInteractionWidget::SetInteractionText(const FText& Text)
{
	if (InteractionTextBlock)
	{
		InteractionTextBlock->SetText(Text);
	}
}
