// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryWarehouseImporter.h"

#include "Items/FactoryItemVisual.h"
#include "Logistics/FactoryInputPortComponent.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"


AFactoryWarehouseImporter::AFactoryWarehouseImporter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFactoryWarehouseImporter::PlanCycle()
{
}

void AFactoryWarehouseImporter::ExecuteCycle()
{
	if (!LogisticsInputPortArr.IsValidIndex(0) || !LogisticsInputPortArr[0]) return;
	
	UFactoryInputPortComponent* InputPort = LogisticsInputPortArr[0];
	
	if (InputPort->PendingItem.ItemData == nullptr) return;
	
	if (UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>())
	{
		WarehouseSubsystem->AddItem(InputPort->PendingItem.ItemData, 1);
		
		if (AFactoryItemVisual* ItemVisual = InputPort->PendingItem.VisualActor.Get())
		{
			if (UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>())
			{
				PoolSubsystem->ReturnItemToPool(ItemVisual);
			}
			else
			{
				ItemVisual->Destroy();
			}
			
			InputPort->PendingItem = FFactoryItemInstance();
		}
	}
}

void AFactoryWarehouseImporter::UpdateView()
{
}

bool AFactoryWarehouseImporter::CanPushItemFromBeforeObject(const UFactoryInputPortComponent* RequestPort) const
{
	if (!RequestPort) return false;
	
	return RequestPort->PendingItem.ItemData == nullptr;
}
