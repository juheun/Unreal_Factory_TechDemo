// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryLogisticsObjectBase.h"

#include "Subsystems/FactoryCycleSubsystem.h"
#include "Logistics/FactoryLogisticsPortComponent.h"


// Sets default values
AFactoryLogisticsObjectBase::AFactoryLogisticsObjectBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFactoryLogisticsObjectBase::InitObject(const class UFactoryObjectData* Data)
{
	Super::InitObject(Data);
	
	InitializeLogisticsPort();
}

/**
 * 공장 시스템의 처리단위마다 호출되는 함수
 */
void AFactoryLogisticsObjectBase::OnFactoryCycle()
{
}

void AFactoryLogisticsObjectBase::InitializeLogisticsPort()
{
	LogisticsPortArr.Empty();
	GetComponents<UFactoryLogisticsPortComponent>(LogisticsPortArr);
	
	for (UFactoryLogisticsPortComponent* Port : LogisticsPortArr)
	{
		if (Port)
		{
			Port->PortOwner = this;
			Port->ScanForConnection();
		}
	}
}

void AFactoryLogisticsObjectBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (UFactoryCycleSubsystem* CycleSubsystem = GetWorld()->GetSubsystem<UFactoryCycleSubsystem>())
	{
		CycleSubsystem->RegisterLogisticsActor(this);
	}
}

void AFactoryLogisticsObjectBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFactoryCycleSubsystem* CycleSubsystem = GetWorld()->GetSubsystem<UFactoryCycleSubsystem>())
	{
		CycleSubsystem->UnregisterLogisticsActor(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

