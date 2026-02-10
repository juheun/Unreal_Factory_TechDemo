// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/FactoryCycleSubsystem.h"
#include "Logistics/FactoryLogisticsObjectBase.h"

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

void UFactoryCycleSubsystem::SortRegisteredLogisticsObjectArr()
{
	// TODO : RegisteredLogisticsObjectArr의 위상정렬 구현
}

void UFactoryCycleSubsystem::OnFactoryCycle()
{
	for (AFactoryLogisticsObjectBase* LogisticsObject : RegisteredLogisticsObjectArr)
	{
		if (IsValid(LogisticsObject))
		{
			LogisticsObject->PlanCycle();
		}
	}
	
	for (AFactoryLogisticsObjectBase* LogisticsObject : RegisteredLogisticsObjectArr)
	{
		if (IsValid(LogisticsObject))
		{
			LogisticsObject->ExecuteCycle();
		}
	}
	
	for (AFactoryLogisticsObjectBase* LogisticsObject : RegisteredLogisticsObjectArr)
	{
		if (IsValid(LogisticsObject))
		{
			LogisticsObject->UpdateView();
		}
	}
}
