// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/FactoryQuickSlotComponent.h"

#include "Player/FactoryPlayerController.h"


UFactoryQuickSlotComponent::UFactoryQuickSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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

UFactoryObjectData* UFactoryQuickSlotComponent::GetQuickSlotData(int Index)
{
	return QuickSlotObjectDataArr.IsValidIndex(Index) ? QuickSlotObjectDataArr[Index] : nullptr;
}



