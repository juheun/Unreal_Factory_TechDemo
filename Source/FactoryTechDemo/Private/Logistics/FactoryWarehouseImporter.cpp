// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryWarehouseImporter.h"

#include "Items/FactoryItemVisual.h"
#include "Logistics/FactoryInputPortComponent.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"
#include "UI/World/FactoryRecipeBillboardComponent.h"
#include "UI/World/FactorySmartNameplateComponent.h"


AFactoryWarehouseImporter::AFactoryWarehouseImporter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SmartNameplateComponent = CreateDefaultSubobject<UFactorySmartNameplateComponent>(TEXT("SartNameplateComponent"));
	SmartNameplateComponent->SetupAttachment(RootComponent);
	
	RecipeBillboardComponent = CreateDefaultSubobject<UFactoryRecipeBillboardComponent>(TEXT("RecipeBillboardComponent"));
	RecipeBillboardComponent->SetupAttachment(RootComponent);
}


void AFactoryWarehouseImporter::BeginPlay()
{
	Super::BeginPlay();
	
	if (RecipeBillboardComponent)
	{
		OnImportItemChanged.AddDynamic(RecipeBillboardComponent, &UFactoryRecipeBillboardComponent::OnItemChangedCallback);
		RecipeBillboardComponent->OnItemChangedCallback(nullptr);
	}
}

void AFactoryWarehouseImporter::InitObject(const UFactoryObjectData* Data)
{
	Super::InitObject(Data);
	
	// 설비 이름 표시
	if (SmartNameplateComponent)
	{
		SmartNameplateComponent->InitNameplate(Data);
	}
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
		
		if (!CachedLastImportedItem || CachedLastImportedItem != InputPort->PendingItem.ItemData)
		{
			CachedLastImportedItem = InputPort->PendingItem.ItemData;
			OnImportItemChanged.Broadcast(InputPort->PendingItem.ItemData);
		}
		
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

bool AFactoryWarehouseImporter::CanPushItemFromBeforeObject(
	const UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem) const
{
	if (!RequestPort || !IncomingItem) return false;
	
	return RequestPort->PendingItem.ItemData == nullptr;
}

