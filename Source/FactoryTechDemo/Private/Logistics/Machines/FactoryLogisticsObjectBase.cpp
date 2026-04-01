#include "Logistics/Machines/FactoryLogisticsObjectBase.h"

#include "Logistics/Ports/FactoryInputPortComponent.h"
#include "Logistics/Ports/FactoryOutputPortComponent.h"
#include "Subsystems/FactoryCycleSubsystem.h"
#include "Logistics/Ports/FactoryPortComponentBase.h"


AFactoryLogisticsObjectBase::AFactoryLogisticsObjectBase()
{
	PrimaryActorTick.bCanEverTick = false;
}


void AFactoryLogisticsObjectBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	InitializeLogisticsPort();
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

int32 AFactoryLogisticsObjectBase::GetConnectedOutputPortNumber() const
{
	int32 ConnectedOutputPortNumber = 0;
	for (int i = 0; i < LogisticsOutputPortArr.Num(); i++)
	{
		if (LogisticsOutputPortArr[i]->GetConnectedInput())
		{
			ConnectedOutputPortNumber++;
		}
	}
	return ConnectedOutputPortNumber;
}

TArray<AFactoryLogisticsObjectBase*> AFactoryLogisticsObjectBase::GetConnectedInputPortsObject() const
{
	TArray<AFactoryLogisticsObjectBase*> ConnectedInputPortObjects;
	for (int i = 0; i < LogisticsInputPortArr.Num(); i++)
	{
		if (LogisticsInputPortArr[i] != nullptr && LogisticsInputPortArr[i]->GetConnectedOutput())
		{
			AFactoryLogisticsObjectBase* Temp = LogisticsInputPortArr[i]->GetConnectedOutput()->GetPortOwner();
			ConnectedInputPortObjects.Add(Temp);
		}
	}
	return ConnectedInputPortObjects;
}

void AFactoryLogisticsObjectBase::InitializeLogisticsPort()
{
	LogisticsOutputPortArr.Empty();
	LogisticsInputPortArr.Empty();
	
	TArray<UFactoryPortComponentBase*> TempPortArr;
	GetComponents<UFactoryPortComponentBase>(TempPortArr);
	
	// 이름에 따라 포트 순서가 자동으로 배열에 잘 들어가도록 정렬
	TempPortArr.Sort([](const UFactoryPortComponentBase& Comp1,const UFactoryPortComponentBase& Comp2)
	{
		return Comp1.GetName() < Comp2.GetName();
	});
	
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


void AFactoryLogisticsObjectBase::SetFacilityBlocked(bool bNewBlock)
{
	if (bIsFacilityBlocked != bNewBlock)
	{
		bIsFacilityBlocked = bNewBlock;
		OnFacilityBlockedStateChanged.Broadcast(bIsFacilityBlocked);
	}
}

