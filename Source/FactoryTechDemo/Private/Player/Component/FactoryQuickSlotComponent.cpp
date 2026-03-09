// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/FactoryQuickSlotComponent.h"

#include "EnhancedInputComponent.h"
#include "Player/FactoryPlayerController.h"
#include "Player/Input/FactoryInputConfig.h"


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
		// TODO : 퀵슬롯 위젯 배치
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



