// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryBeltBridge.h"

#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"


AFactoryBeltBridge::AFactoryBeltBridge()
{
	PrimaryActorTick.bCanEverTick = false;
	
	LogisticsObjectType = ELogisticsObjectType::Conveyor;
}

void AFactoryBeltBridge::BeginPlay()
{
	Super::BeginPlay();
	
	if (LogisticsInputPortArr.Num() != 4 || LogisticsOutputPortArr.Num() != 4)
	{
		UE_LOG(LogTemp, Error, TEXT("[AFactoryBeltBridge] 포트 개수가 4개가 아닙니다! BP 세팅을 확인하세요. 현재 In: %d, Out: %d"), LogisticsInputPortArr.Num(), LogisticsOutputPortArr.Num());
		return;
	}
	
	CurrentItems.SetNum(4);
	
	// 포트가 마음대로 설치시 연결이 됐다면 강제로 연결을 끊어 브릿지가 통제 및 델리게이트 등록
	for (int i = 0; i < 4; i++)
	{
		if (LogisticsOutputPortArr[i])
		{
			LogisticsOutputPortArr[i]->Disconnect();
			LogisticsOutputPortArr[i]->OnPortConnectionChanged.AddDynamic(this, &AFactoryBeltBridge::HandlePortConnectionChanged);
		}
		if (LogisticsInputPortArr[i])
		{
			LogisticsInputPortArr[i]->Disconnect();
			LogisticsInputPortArr[i]->OnPortConnectionChanged.AddDynamic(this, &AFactoryBeltBridge::HandlePortConnectionChanged);
		}
	}
	
	// 포트 기본 상태로 세팅
	for (int i = 0; i < 4; i++)
	{
		if (LogisticsOutputPortArr[i]) LogisticsOutputPortArr[i]->SetPortEnabled(false);
	}
	
	// 인풋 포트 순회하며 Connect검사
	for (int i = 0; i < 4; i++)
	{
		if (LogisticsOutputPortArr[i] && LogisticsOutputPortArr[i]->GetIsPortEnabled())
		{
			continue;
		}
		if (LogisticsInputPortArr[i]) LogisticsInputPortArr[i]->ForceScanConnection();
	}
}

void AFactoryBeltBridge::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
	if (!WarehouseSubsystem) return;
	
	UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
	if (!PoolSubsystem) return;
	
	for (int i = 0; i < CurrentItems.Num(); i++)
	{
		if (CurrentItems[i].IsValid())
		{
			WarehouseSubsystem->AddItem(
				const_cast<UFactoryItemData*>(CurrentItems[i].ItemData.Get()), 1);
		}
	}
	
	//Pending된 아이템도 제거
	for (UFactoryInputPortComponent* Port : LogisticsInputPortArr)
	{
		if (Port && Port->PendingItem.IsValid())
		{
			WarehouseSubsystem->AddItem(
				const_cast<UFactoryItemData*>(Port->PendingItem.ItemData.Get()), 1);
		}
	}
}

void AFactoryBeltBridge::PlanCycle()
{
	UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
	if (!PoolSubsystem) return;
	
	for (int i = 0; i < 4; i++)
	{
		int32 Opposite = GetOppositePortIndex(i);
		
		if (!CurrentItems[i].IsValid() || !LogisticsOutputPortArr[Opposite]) continue;
		
		UFactoryInputPortComponent* TargetPort = LogisticsOutputPortArr[Opposite]->GetConnectedInput();
		if (!TargetPort) continue;
		
		if (TargetPort->GetPortOwner()->CanPushItemFromBeforeObject(TargetPort, CurrentItems[i].ItemData))
		{
			FFactoryItemInstance NewInstance(CurrentItems[i].ItemData);
			TargetPort->PendingItem = NewInstance;
			CurrentItems[i] = FFactoryItemInstance();
		}
	}
}

void AFactoryBeltBridge::LatePlanCycle()
{
	PlanCycle();
}

void AFactoryBeltBridge::ExecuteCycle()
{
	for (int i = 0; i < 4; i++)
	{
		if (!LogisticsInputPortArr.IsValidIndex(i) || !LogisticsInputPortArr[i]) continue;
		if (!LogisticsInputPortArr[i]->PendingItem.IsValid()) continue;
		
		if (!CurrentItems[i].IsValid())
		{
			CurrentItems[i] = LogisticsInputPortArr[i]->PendingItem;
			LogisticsInputPortArr[i]->PendingItem = FFactoryItemInstance();
		}
	}
}

void AFactoryBeltBridge::UpdateView()
{
	// 애니메이션 재생 등
}

bool AFactoryBeltBridge::CanPushItemFromBeforeObject(
	UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem)
{
	if (!RequestPort || RequestPort->PendingItem.IsValid()) return false;	// Pending 되어 있지 않아야 밀어넣을 수 있음
	if (!IncomingItem) return false;
	
	int32 PortIndex = LogisticsInputPortArr.IndexOfByKey(RequestPort);
	
	if (PortIndex != INDEX_NONE && CurrentItems.IsValidIndex(PortIndex))
	{
		return !CurrentItems[PortIndex].IsValid();
	}
	
	return false;
}

bool AFactoryBeltBridge::PullItemFromInputPorts(FFactoryItemInstance& Item)
{
	// PlanCycle에서 직접 구현하여 별도로 PullItemFromInputPorts를 구현하지 않음
	return false;
}

void AFactoryBeltBridge::HandlePortConnectionChanged(UFactoryPortComponentBase* Port, bool bIsConnected)
{
	// 입력 포트가 연결/해제될 때만 반응하여 반대편 출구를 제어
	if (UFactoryInputPortComponent* InPort = Cast<UFactoryInputPortComponent>(Port))
	{
		int32 Index = LogisticsInputPortArr.IndexOfByKey(InPort);
		int32 Opposite = GetOppositePortIndex(Index);

		if (Index != INDEX_NONE && Opposite != INDEX_NONE)
		{
			if (bIsConnected)
			{
				// 반대편의 Input을 막고 Output을 연다
				LogisticsInputPortArr[Opposite]->SetPortEnabled(false);
				LogisticsOutputPortArr[Opposite]->SetPortEnabled(true);
			}
			else
			{
				// 반대편의 Output을 닫고 Input을 다시 연다
				LogisticsOutputPortArr[Opposite]->SetPortEnabled(false);
				LogisticsInputPortArr[Opposite]->SetPortEnabled(true);
			}
		}
	}
}

int32 AFactoryBeltBridge::GetOppositePortIndex(int32 PortIndex) const
{
	// 0(+X) ↔ 1(-X), 2(+Y) ↔ 3(-Y)
	if (PortIndex == 0) return 1;
	if (PortIndex == 1) return 0;
	if (PortIndex == 2) return 3;
	if (PortIndex == 3) return 2;
	return INDEX_NONE;
}



