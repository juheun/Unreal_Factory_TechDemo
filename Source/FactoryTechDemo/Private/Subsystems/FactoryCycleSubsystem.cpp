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
	TArray<TWeakObjectPtr<AFactoryLogisticsObjectBase>> SortedOutportArray;
	
	// 초기화. 연결된 outport가 0인 설비 골라서 Enqueue
	for (int i = 0; i < RegisteredLogisticsObjectArr.Num(); i++)
	{
		if (AFactoryLogisticsObjectBase* RegisteredObjPtr = RegisteredLogisticsObjectArr[i].Get())
		{
			int32 ConnectedOutputPortNumber = RegisteredObjPtr->GetConnectedOutputPortNumber();
			if (ConnectedOutputPortNumber == 0)
			{
				ZeroOutportQueue.Enqueue(RegisteredObjPtr);
			}
			else
			{
				OutportConnectedMap.Add(RegisteredObjPtr, ConnectedOutputPortNumber);
			}
		}
	}
	
	int32 SafetyCounter = 0;
	const int32 MaxIterations = RegisteredLogisticsObjectArr.Num() * 2;
	
	while (SortedOutportArray.Num() < RegisteredLogisticsObjectArr.Num() && SafetyCounter < MaxIterations)
	{
		SafetyCounter++;
		
		AFactoryLogisticsObjectBase* DequeuedObject = nullptr;
		if (ZeroOutportQueue.Dequeue(DequeuedObject))
		{
			// Queue에 남은 Object 있는 경우 kahn 알고리즘 지속
			TArray<AFactoryLogisticsObjectBase*> ConnectedObject = DequeuedObject->GetConnectedInputPortsObject();
			for (int i = 0; i < ConnectedObject.Num(); i++)
			{
				// 맵에 해당 설비가 있는지 안전하게 확인
				if (int32* OutDegreePtr = OutportConnectedMap.Find(ConnectedObject[i]))
				{
					// 포인터를 통해 값을 직접 수정
					(*OutDegreePtr)--;

					if (*OutDegreePtr <= 0)
					{
						ZeroOutportQueue.Enqueue(ConnectedObject[i]);
						OutportConnectedMap.Remove(ConnectedObject[i]);
					}
				}
			}
			SortedOutportArray.Add(DequeuedObject);
		}
		else
		{
			// Queue에 남은 Object 없는 경우 순환 설비 존재. 강제로 끊어냄
			AFactoryLogisticsObjectBase* CycleBreaker = nullptr;
			for (auto& Pair : OutportConnectedMap)
			{
				CycleBreaker = Pair.Key;
				break; 
			}
          
			if (CycleBreaker)
			{
				ZeroOutportQueue.Enqueue(CycleBreaker);
				OutportConnectedMap.Remove(CycleBreaker);
			}
		}
	}
	
	//  만약 SafetyCounter가 터졌더라도, 남은 객체들을 증발시키지 않고 억지로라도 배열에 넣음
	for (auto& Pair : OutportConnectedMap)
	{
		SortedOutportArray.Add(Pair.Key);
	}
	
	// 정렬된 배열로 갈아끼움
	RegisteredLogisticsObjectArr = SortedOutportArray;
}

void UFactoryCycleSubsystem::OnFactoryCycle()
{
	if (!GetWorld() || !GetWorld()->IsGameWorld()) return;
	
	//죽은 포인터 전부 청소
	int32 RemovedCount = RegisteredLogisticsObjectArr.RemoveAllSwap(
		[](const TWeakObjectPtr<AFactoryLogisticsObjectBase>& Ptr)
		{
			return !Ptr.IsValid();
		});
	// 하나라도 삭제되면 재정렬 
	if (RemovedCount > 0)
	{
		bGraphDirty = true;
	}
	
	if(bGraphDirty)
	{
		SortRegisteredLogisticsObjectArr();
		bGraphDirty = false;
	}
	
	LastStartCycleTime = GetWorld()->GetTimeSeconds();

	for (const auto& WeakObjectPtr : RegisteredLogisticsObjectArr)
	{
		if (AFactoryLogisticsObjectBase* LogisticsObject =  WeakObjectPtr.Get())
		{
			LogisticsObject->PlanCycle();
		}
	}
	
	for (const auto& WeakObjectPtr : RegisteredLogisticsObjectArr)
	{
		if (AFactoryLogisticsObjectBase* LogisticsObject =  WeakObjectPtr.Get())
		{
			LogisticsObject->LatePlanCycle();
		}
	}
	
	for (const auto& WeakObjectPtr : RegisteredLogisticsObjectArr)
	{
		if (AFactoryLogisticsObjectBase* LogisticsObject =  WeakObjectPtr.Get())
		{
			LogisticsObject->ExecuteCycle();
		}
	}
	
	for (const auto& WeakObjectPtr : RegisteredLogisticsObjectArr)
	{
		if (AFactoryLogisticsObjectBase* LogisticsObject =  WeakObjectPtr.Get())
		{
			LogisticsObject->UpdateView();
		}
	}
}
