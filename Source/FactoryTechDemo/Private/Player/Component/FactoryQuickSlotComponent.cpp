// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/FactoryQuickSlotComponent.h"

#include "EnhancedInputComponent.h"
#include "Player/FactoryPlayerController.h"
#include "Player/Input/FactoryInputConfig.h"
#include "UI/QuickSlot/FactoryQuickBarWidget.h"


UFactoryQuickSlotComponent::UFactoryQuickSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFactoryQuickSlotComponent::SetUpInputComponent(UEnhancedInputComponent* PlayerInputComp,
	const UFactoryInputConfig* InputConfig)
{
	for (int i = 0; i < InputConfig->QuickSlotActionArr.Num(); i++)
	{
		if (InputConfig->QuickSlotActionArr[i])
		{
			PlayerInputComp->BindAction(InputConfig->QuickSlotActionArr[i], ETriggerEvent::Started, this, &UFactoryQuickSlotComponent::ExecuteQuickSlotData, i);
		}
	}
}

void UFactoryQuickSlotComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CachedPlayerController = Cast<AFactoryPlayerController>(GetOwner());
	if (AFactoryPlayerController* Controller = CachedPlayerController.Get())
	{
		if (Controller->IsLocalController() && QuickBarWidgetBP)
		{
			QuickBarWidget = CreateWidget<UFactoryQuickBarWidget>(Controller, QuickBarWidgetBP);
			if (QuickBarWidget)
			{
				QuickBarWidget->AddToViewport();
				QuickBarWidget->InitQuickBar(this);
			}
		}
	}
	
	if (QuickSlotObjectDataArr.Num() < 10)
	{
		QuickSlotObjectDataArr.SetNum(10);
	}
}

void UFactoryQuickSlotComponent::SetQuickSlotData(int32 Index, UFactoryObjectData* InData)
{
	if (QuickSlotObjectDataArr.IsValidIndex(Index))
	{
		QuickSlotObjectDataArr[Index] = InData;
		OnQuickSlotDataChanged.Broadcast(Index, InData); // UI 갱신 방송!
	}
}

void UFactoryQuickSlotComponent::ExecuteQuickSlotData(int Index)
{
	UFactoryObjectData* ObjectData = QuickSlotObjectDataArr.IsValidIndex(Index) ? QuickSlotObjectDataArr[Index] : nullptr;
	if (ObjectData)
	{
		OnQuickSlotExecuted.Broadcast(ObjectData);
	}
}



