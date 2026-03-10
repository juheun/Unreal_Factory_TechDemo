// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryBeltRouter.h"

#include "Items/FactoryItemVisual.h"
#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Settings/FactoryDeveloperSettings.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"


class UFactoryPoolSubsystem;

AFactoryBeltRouter::AFactoryBeltRouter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFactoryBeltRouter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
	if (!WarehouseSubsystem) return;
	
	UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
	if (!PoolSubsystem) return;
	
	if (CurrentItem.IsValid())
	{
		WarehouseSubsystem->AddItem(
			const_cast<UFactoryItemData*>(CurrentItem.ItemData.Get()), 1);
		if (CurrentItem.VisualActor.Get())
		{
			PoolSubsystem->ReturnItemToPool(CurrentItem.VisualActor.Get()); 
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

void AFactoryBeltRouter::PlanCycle()
{
	if (!CurrentItem.ItemData) return;
	
	int32 MaxOutputPorts = LogisticsOutputPortArr.Num();
	if (MaxOutputPorts <= 0) return;
	
	int32 OutputPortNum = CurrentOutputIndex;
	for (int i = 0; i < MaxOutputPorts; i++)
	{
		int index = (OutputPortNum + i) % MaxOutputPorts;
		if (!LogisticsOutputPortArr.IsValidIndex(index) || !LogisticsOutputPortArr[index]) continue;
		
		UFactoryInputPortComponent* TargetPort = LogisticsOutputPortArr[index]->GetConnectedInput();
		if (!TargetPort) continue;
		
		if (TargetPort->GetPortOwner()->CanPushItemFromBeforeObject(TargetPort))
		{
			UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
			if (!PoolSubsystem) return;
			
			FFactoryItemInstance NewInstance(CurrentItem.ItemData);
			FVector SpawnLocation = LogisticsOutputPortArr[index]->GetComponentLocation();	// 현재 내 Output 포트 위치에 스폰
			FRotator SpawnRotation = LogisticsOutputPortArr[index]->GetComponentRotation();
			AFactoryItemVisual* ItemVisual = PoolSubsystem->GetItemFromPool<AFactoryItemVisual>(
				EFactoryPoolType::ItemVisual, SpawnLocation, SpawnRotation);
			ItemVisual->UpdateVisual(CurrentItem.ItemData);
			if (ItemVisual)
			{
				NewInstance.VisualActor = ItemVisual;
			}
			
			TargetPort->PendingItem = NewInstance;
			CurrentItem = FFactoryItemInstance();
			CurrentOutputIndex = (index + 1) % MaxOutputPorts;
			break;
		}
	}
}

void AFactoryBeltRouter::ExecuteCycle()
{
	int32 MaxInputPorts = LogisticsInputPortArr.Num();
	if (MaxInputPorts <= 0) return;
	
	int32 InputPortNum = CurrentInputIndex;
	for (int i = 0; i < MaxInputPorts; i++)
	{
		int index = (InputPortNum + i) % MaxInputPorts;
		
		if (!LogisticsInputPortArr.IsValidIndex(index) || !LogisticsInputPortArr[index]) continue;
		if (LogisticsInputPortArr[index]->PendingItem.IsValid())
		{
			if (PullItemFromInputPorts(LogisticsInputPortArr[index]->PendingItem))
			{
				if (AFactoryItemVisual* VisualActor = LogisticsInputPortArr[index]->PendingItem.VisualActor.Get())
				{
					UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
					if (!PoolSubsystem) return;
					
					PoolSubsystem->ReturnItemToPool(VisualActor);
				}
				LogisticsInputPortArr[index]->PendingItem = FFactoryItemInstance();
				CurrentInputIndex = (index + 1) % MaxInputPorts;
				
				break;
			}
		}
	}
}

void AFactoryBeltRouter::UpdateView()
{
	// 애니메이션 재생 등
}

bool AFactoryBeltRouter::CanPushItemFromBeforeObject(const UFactoryInputPortComponent* RequestPort) const
{
	if (!RequestPort || RequestPort->PendingItem.IsValid()) return false;	// Pending 되어 있지 않아야 밀어넣을 수 있음
	const UFactoryItemData* PendingItem = RequestPort->PendingItem.ItemData;
	return !PendingItem && !CurrentItem.ItemData;
}

bool AFactoryBeltRouter::PullItemFromInputPorts(FFactoryItemInstance& Item)
{
	CurrentItem = Item;
	return true;
}

