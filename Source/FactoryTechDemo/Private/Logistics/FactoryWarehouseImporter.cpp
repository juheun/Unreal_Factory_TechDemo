// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryWarehouseImporter.h"

#include "Items/FactoryItemVisual.h"
#include "Logistics/FactoryInputPortComponent.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"
#include "UI/World/FactoryPortBlockWarningComponent.h"
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
	
	UFactoryPortBlockWarningComponent* PortBlockWarningComponent = NewObject<UFactoryPortBlockWarningComponent>(this);
	PortBlockWarningComponent->SetupAttachment(LogisticsInputPortArr[0]);
	PortBlockWarningComponent->RegisterComponent();
	LogisticsInputPortArr[0]->OnPortBlockedStateChanged.AddDynamic(PortBlockWarningComponent, &UFactoryPortBlockWarningComponent::OnPortBlockedCallback);
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
	
	if (InputPort->PendingItem.ItemData == nullptr)
	{
		// 포트에 펜딩된 아이템 없으면 막힌게 아님
		InputPort->SetPortBlocked(false);
		return;
	}
	
	if (UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>())
	{
		int ItemAmountBeforeAdd = WarehouseSubsystem->GetItemAmount(InputPort->PendingItem.ItemData);
		int ItemAmountAfterAdd = WarehouseSubsystem->AddItem(InputPort->PendingItem.ItemData, 1);
		
		if (ItemAmountBeforeAdd < ItemAmountAfterAdd)
		{
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
			InputPort->SetPortBlocked(false);
		}
		else
		{
			InputPort->SetPortBlocked(true);
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

