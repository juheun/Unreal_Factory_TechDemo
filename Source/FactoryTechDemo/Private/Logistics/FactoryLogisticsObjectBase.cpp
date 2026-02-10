// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryLogisticsObjectBase.h"

#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Subsystems/FactoryCycleSubsystem.h"
#include "Logistics/FactoryPortComponentBase.h"


// Sets default values
AFactoryLogisticsObjectBase::AFactoryLogisticsObjectBase()
{
	PrimaryActorTick.bCanEverTick = false;
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

void AFactoryLogisticsObjectBase::InitObject(const class UFactoryObjectData* Data)
{
	Super::InitObject(Data);
	
	InitializeLogisticsPort();
}

/**
 * 다음에 행할 상태를 정함
 */
void AFactoryLogisticsObjectBase::DetermineNextState()
{
	
}

/**
 * 예약된 상태대로 아이템을 옮김
 */
void AFactoryLogisticsObjectBase::CommitState()
{
	
}

/**
 * 업데이트된 상태대로 외형을 업데이트함
 */
void AFactoryLogisticsObjectBase::UpdateState()
{
	
}


void AFactoryLogisticsObjectBase::InitializeLogisticsPort()
{
	LogisticsOutputPortArr.Empty();
	LogisticsInputPortArr.Empty();
	
	TArray<UFactoryPortComponentBase*> TempPortArr;
	GetComponents<UFactoryPortComponentBase>(TempPortArr);
	TempPortArr.Sort([](const UFactoryPortComponentBase& Comp1,const UFactoryPortComponentBase& Comp2)
	{
		return Comp1.GetName() < Comp2.GetName();
	});		// 이름에 따라 포트 순서가 자동으로 배열에 잘 들어가도록 정렬
	
	for (UFactoryPortComponentBase* Port : TempPortArr)
	{
		if (Port)
		{
			if (UFactoryInputPortComponent* InputPort = Cast<UFactoryInputPortComponent>(Port))
			{
				LogisticsInputPortArr.Add(InputPort);
			}
			else if (UFactoryOutputPortComponent* OutputPort = Cast<UFactoryOutputPortComponent>(Port))
			{
				LogisticsOutputPortArr.Add(OutputPort);
			}
		}
	}
}