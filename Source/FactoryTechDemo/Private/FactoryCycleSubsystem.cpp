// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryCycleSubsystem.h"
#include "FactoryLogisticsObjectBase.h"

void UFactoryCycleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	GetWorld()->GetTimerManager().SetTimer(
		CycleTimerHandle,
		this,
		&UFactoryCycleSubsystem::OnFactoryCycle,
		CycleInterval,
		true);
}

void UFactoryCycleSubsystem::Deinitialize()
{
	GetWorld()->GetTimerManager().ClearTimer(CycleTimerHandle);
	Super::Deinitialize();
}

void UFactoryCycleSubsystem::RegisterLogisticsActor(class AFactoryLogisticsObjectBase* LogisticsObject)
{
	if (LogisticsObject)
	{
		RegisteredLogisticsObjectArr.AddUnique(LogisticsObject);
	}
}

void UFactoryCycleSubsystem::UnregisterLogisticsActor(class AFactoryLogisticsObjectBase* LogisticsObject)
{
	if (LogisticsObject)
	{
		RegisteredLogisticsObjectArr.Remove(LogisticsObject);
	}
}

void UFactoryCycleSubsystem::OnFactoryCycle()
{
	for (AFactoryLogisticsObjectBase* LogisticsObject : RegisteredLogisticsObjectArr)
	{
		if (IsValid(LogisticsObject))
		{
			LogisticsObject->OnFactoryCycle();
		}
	}
}
