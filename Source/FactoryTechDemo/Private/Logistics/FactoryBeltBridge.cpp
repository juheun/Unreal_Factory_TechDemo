// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryBeltBridge.h"

#include "Items/FactoryItemVisual.h"
#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"


AFactoryBeltBridge::AFactoryBeltBridge()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFactoryBeltBridge::BeginPlay()
{
	Super::BeginPlay();
	
	if (LogisticsOutputPortArr.Num() != LaneCount || LogisticsInputPortArr.Num() != LaneCount)
	{
		UE_LOG(LogTemp, Error, TEXT("BeltBridge Port Num Error"));
		return;
	}
	CurrentItems.SetNum(LaneCount);
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
			if (CurrentItems[i].VisualActor.Get())
			{
				PoolSubsystem->ReturnItemToPool(CurrentItems[i].VisualActor.Get()); 
			}
		}
	}
	
	//Pending된 아이템도 제거
	for (UFactoryInputPortComponent* Port : LogisticsInputPortArr)
	{
		if (Port && Port->PendingItem.IsValid())
		{
			WarehouseSubsystem->AddItem(
				const_cast<UFactoryItemData*>(Port->PendingItem.ItemData.Get()), 1);
			if (Port->PendingItem.VisualActor.IsValid())
			{
				PoolSubsystem->ReturnItemToPool(Port->PendingItem.VisualActor.Get()); 
			}
		}
	}
}

void AFactoryBeltBridge::PlanCycle()
{
	UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
	if (!PoolSubsystem) return;
	
	for (int i = 0; i < LaneCount; i++)
	{
		if (!CurrentItems[i].IsValid() || !LogisticsOutputPortArr.IsValidIndex(i) || !LogisticsOutputPortArr[i]) continue;
		
		UFactoryInputPortComponent* TargetPort = LogisticsOutputPortArr[i]->GetConnectedInput();
		if (!TargetPort) continue;
		
		if (TargetPort->GetPortOwner()->CanPushItemFromBeforeObject(TargetPort, CurrentItems[i].ItemData))
		{
			FFactoryItemInstance NewInstance(CurrentItems[i].ItemData);
			FVector SpawnLocation = LogisticsOutputPortArr[i]->GetComponentLocation();	// 현재 내 Output 포트 위치에 스폰
			FRotator SpawnRotation = LogisticsOutputPortArr[i]->GetComponentRotation();
			AFactoryItemVisual* ItemVisual = PoolSubsystem->GetItemFromPool<AFactoryItemVisual>(
				EFactoryPoolType::ItemVisual, SpawnLocation, SpawnRotation);
			ItemVisual->UpdateVisual(CurrentItems[i].ItemData);
			if (ItemVisual)
			{
				NewInstance.VisualActor = ItemVisual;
			}
			
			TargetPort->PendingItem = NewInstance;
			CurrentItems[i] = FFactoryItemInstance();
		}
	}
}

void AFactoryBeltBridge::ExecuteCycle()
{
	for (int i = 0; i < LaneCount; i++)
	{
		if (!LogisticsInputPortArr.IsValidIndex(i) || !LogisticsInputPortArr[i]) continue;
		if (!LogisticsInputPortArr[i]->PendingItem.IsValid()) continue;
		
		if (!CurrentItems[i].IsValid())
		{
			CurrentItems[i] = LogisticsInputPortArr[i]->PendingItem;
			
			if (AFactoryItemVisual* VisualActor = CurrentItems[i].VisualActor.Get())
			{
				if (UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>())
				{
					PoolSubsystem->ReturnItemToPool(VisualActor);
				}
			}
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



