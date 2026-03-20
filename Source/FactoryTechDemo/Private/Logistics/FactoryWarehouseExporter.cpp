// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryWarehouseExporter.h"

#include "Items/FactoryItemVisual.h"
#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"


AFactoryWarehouseExporter::AFactoryWarehouseExporter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	FacilityMenuMode = EFactoryMenuMode::FacilityOnly;
}

void AFactoryWarehouseExporter::PlanCycle()
{
	// 벨트는 무조건 Output이 하나라는 가정하에 index 0사용
	if (!TargetItemData || !LogisticsOutputPortArr.IsValidIndex(0) || !LogisticsOutputPortArr[0]) return;
	UFactoryInputPortComponent* TargetPort = LogisticsOutputPortArr[0]->GetConnectedInput();
	if (!TargetPort) return;
	if (UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>())
	{
		if (TargetPort->GetPortOwner()->CanPushItemFromBeforeObject(TargetPort, TargetItemData))
		{
			if (WarehouseSubsystem->TryRemoveItem(TargetItemData, 1))
			{
				UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
				if (!PoolSubsystem) return;
				
				FFactoryItemInstance NewInstance(TargetItemData);
				FVector SpawnLocation = TargetPort->GetComponentLocation(); // 일단 포트 위치에 스폰
				FRotator SpawnRotation = TargetPort->GetComponentRotation();
				AFactoryItemVisual* ItemVisual = 
					PoolSubsystem->GetItemFromPool<AFactoryItemVisual>(EFactoryPoolType::ItemVisual, SpawnLocation, SpawnRotation);
				if (ItemVisual)
				{
					ItemVisual->UpdateVisual(TargetItemData);
					NewInstance.VisualActor = ItemVisual;
					TargetPort->PendingItem = NewInstance;	// 상대방 Input에 아이템 밀어넣기
				}
			}
		}
		int32 WarehouseAmount = WarehouseSubsystem->GetItemAmount(TargetItemData);
		OnWarehouseAmountUpdated.Broadcast(WarehouseAmount);
	}
}

void AFactoryWarehouseExporter::ExecuteCycle()
{
}

void AFactoryWarehouseExporter::UpdateView()
{
}

bool AFactoryWarehouseExporter::CanPushItemFromBeforeObject(const UFactoryInputPortComponent* RequestPort,
	const UFactoryItemData* IncomingItem) const
{
	return false;
}

bool AFactoryWarehouseExporter::PullItemFromInputPorts(FFactoryItemInstance& Item)
{
	return false;
}

void AFactoryWarehouseExporter::SetTargetItem(UFactoryItemData* NewTargetItem)
{
	if (TargetItemData != NewTargetItem)
	{
		TargetItemData = NewTargetItem;
		OnTargetItemChanged.Broadcast(TargetItemData);

		if (UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>())
		{
			int32 WarehouseAmount = WarehouseSubsystem->GetItemAmount(TargetItemData);
			OnWarehouseAmountUpdated.Broadcast(WarehouseAmount);
		}
	}
}
