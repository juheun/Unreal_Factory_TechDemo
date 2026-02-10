#include "Logistics/FactoryLogisticsObjectBase.h"

#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Subsystems/FactoryCycleSubsystem.h"
#include "Logistics/FactoryPortComponentBase.h"


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

void AFactoryLogisticsObjectBase::InitObject(const UFactoryObjectData* Data)
{
	Super::InitObject(Data);
	
	InitializeLogisticsPort();
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