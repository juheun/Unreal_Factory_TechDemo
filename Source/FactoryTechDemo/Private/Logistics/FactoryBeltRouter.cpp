// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryBeltRouter.h"

#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
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
		
		if (TargetPort->GetPortOwner()->CanPushItemFromBeforeObject(TargetPort, CurrentItem.ItemData))
		{
			UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
			if (!PoolSubsystem) return;
			
			FFactoryItemInstance NewInstance(CurrentItem.ItemData);
			TargetPort->PendingItem = NewInstance;
			CurrentItem = FFactoryItemInstance();
			CurrentOutputIndex = (index + 1) % MaxOutputPorts;
			break;
		}
	}
}

void AFactoryBeltRouter::LatePlanCycle()
{
	PlanCycle();
}

void AFactoryBeltRouter::ExecuteCycle()
{
	if (CurrentItem.ItemData) return;
	
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

bool AFactoryBeltRouter::CanPushItemFromBeforeObject(
	UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem)
{
	if (!RequestPort || RequestPort->PendingItem.IsValid()) return false;	// Pending 되어 있지 않아야 밀어넣을 수 있음
	if (!IncomingItem) return false;
	
	const UFactoryItemData* PendingItem = RequestPort->PendingItem.ItemData;
	return !PendingItem && !CurrentItem.ItemData;
}

bool AFactoryBeltRouter::PullItemFromInputPorts(FFactoryItemInstance& Item)
{
	CurrentItem = Item;
	return true;
}

