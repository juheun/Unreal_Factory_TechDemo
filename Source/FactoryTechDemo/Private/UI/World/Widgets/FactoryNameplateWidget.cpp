// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/Widgets/FactoryNameplateWidget.h"

#include "Components/TextBlock.h"

void UFactoryNameplateWidget::SetFacilityName(const FText& InName)
{
	if (FacilityNameText)
	{
		FacilityNameText->SetText(InName);
	}
}
