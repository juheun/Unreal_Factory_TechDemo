// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryLogisticsObjectBase.h"
#include "FactoryLogisticsPortComponent.h"


// Sets default values
AFactoryLogisticsObjectBase::AFactoryLogisticsObjectBase()
{
	//TODO: 디버그 끝나면 false로 바꿀것
	PrimaryActorTick.bCanEverTick = true;
}

void AFactoryLogisticsObjectBase::InitObject(const class UFactoryObjectData* Data)
{
	Super::InitObject(Data);
	
	InitializeLogisticsPort();
}

// Called every frame
void AFactoryLogisticsObjectBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

