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
		bGraphDirty = true;
	}
}

void UFactoryCycleSubsystem::UnregisterLogisticsActor(class AFactoryLogisticsObjectBase* LogisticsObject)
{
	if (LogisticsObject)
	{
		RegisteredLogisticsObjectArr.Remove(LogisticsObject);
		bGraphDirty = true;
	}
}

float UFactoryCycleSubsystem::GetCycleAlpha() const
{
	float NowTime = GetWorld()->GetTimeSeconds();
	float Alpha = (NowTime - LastStartCycleTime) / CycleInterval;
	return FMath::Clamp(Alpha, 0.0f, 1.f);
}

void UFactoryCycleSubsystem::SortRegisteredLogisticsObjectArr()
{
	TMap<AFactoryLogisticsObjectBase*, int32> OutportConnectedMap;
	TQueue<AFactoryLogisticsObjectBase*> ZeroOutportQueue;
	TArray<AFactoryLogisticsObjectBase*> SortedOutportArray;
	
	// 초기화. 연결된 outport가 0인 설비 골라서 Enqueue
	for (int i = 0; i < RegisteredLogisticsObjectArr.Num(); i++)
	{
		int32 ConnectedOutputPortNumber = RegisteredLogisticsObjectArr[i]->GetConnectedOutputPortNumber();
		if (ConnectedOutputPortNumber == 0)
		{
			ZeroOutportQueue.Enqueue(RegisteredLogisticsObjectArr[i]);
		}
		else
		{
			OutportConnectedMap.Add(RegisteredLogisticsObjectArr[i], ConnectedOutputPortNumber);
		}
	}
	
	while (SortedOutportArray.Num() != RegisteredLogisticsObjectArr.Num())
	{
		AFactoryLogisticsObjectBase* DequeuedObject = nullptr;
		if (ZeroOutportQueue.Dequeue(DequeuedObject))
		{
			// Queue에 남은 Object 있는 경우 kahn 알고리즘 지속
			TArray<AFactoryLogisticsObjectBase*> ConnectedObject = DequeuedObject->GetConnectedInputPortsObject();
			for (int i = 0; i < ConnectedObject.Num(); i++)
			{
				OutportConnectedMap[ConnectedObject[i]]--;
				if (OutportConnectedMap[ConnectedObject[i]] == 0)
				{
					ZeroOutportQueue.Enqueue(ConnectedObject[i]);
					OutportConnectedMap.Remove(ConnectedObject[i]);
				}
			}
			SortedOutportArray.Add(DequeuedObject);
		}
		else
		{
			// Queue에 남은 Object 없는 경우 순환 설비 존재. 강제로 끊어낼 설비 찾기 시작
			AFactoryLogisticsObjectBase* CycleStarter = nullptr;
			for (auto& Pair : OutportConnectedMap)
			{
				CycleStarter = Pair.Key;
				break; 
			}
			if (CycleStarter)
			{
				AFactoryLogisticsObjectBase* Current = CycleStarter;
				TSet<AFactoryLogisticsObjectBase*> Visited;
				while (Current && !Visited.Contains(Current))
				{
					Visited.Add(Current);
					const TArray<AFactoryLogisticsObjectBase*> InputObjects = Current->GetConnectedInputPortsObject();
					if (InputObjects.Num() > 0) 
					{
						Current = InputObjects[0];
					}
					else 
					{
						break; 
					}
				}
				ZeroOutportQueue.Enqueue(Current);
				OutportConnectedMap.Remove(Current);
			}
		}
	}
	
	// 정렬된 배열로 갈아끼움
	RegisteredLogisticsObjectArr = SortedOutportArray;
}

void UFactoryCycleSubsystem::OnFactoryCycle()
{
	if(bGraphDirty)
	{
		SortRegisteredLogisticsObjectArr();
		bGraphDirty = false;
	}
	
	LastStartCycleTime = GetWorld()->GetTimeSeconds();

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
