// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryWarehouseExporter.h"

#include "Items/FactoryItemVisual.h"
#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"


// Sets default values
AFactoryWarehouseExporter::AFactoryWarehouseExporter()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AFactoryWarehouseExporter::PlanCycle()
{
	// 벨트는 무조건 Output이 하나라는 가정하에 index 0사용
	if (!ItemData || !LogisticsOutputPortArr.IsValidIndex(0) || !LogisticsOutputPortArr[0]) return;
	UFactoryInputPortComponent* TargetPort = LogisticsOutputPortArr[0]->GetConnectedInput();
	if (!TargetPort) return;
	
	if (TargetPort->GetPortOwner()->CanPushItemFromBeforeObject(TargetPort))
	{
		UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
		if (WarehouseSubsystem && WarehouseSubsystem->TryRemoveItem(ItemData, 1))
		{
			UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
			if (!PoolSubsystem) return;
			
			FFactoryItemInstance NewInstance(ItemData);
			FVector SpawnLocation = TargetPort->GetComponentLocation(); // 일단 포트 위치에 스폰
			FRotator SpawnRotation = TargetPort->GetComponentRotation();
			AFactoryItemVisual* ItemVisual = 
				PoolSubsystem->GetItemFromPool<AFactoryItemVisual>(EFactoryPoolType::ItemVisual, SpawnLocation, SpawnRotation);
			if (ItemVisual)
			{
				ItemVisual->UpdateVisual(ItemData);
				NewInstance.VisualActor = ItemVisual;
				TargetPort->PendingItem = NewInstance;	// 상대방 Input에 아이템 밀어넣기
			}
		}
	}
}

void AFactoryWarehouseExporter::ExecuteCycle()
{
}

void AFactoryWarehouseExporter::UpdateView()
{
}


