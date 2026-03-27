// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryWarehouseImporter.h"

#include "Logistics/FactoryInputPortComponent.h"
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
	
	LogisticsObjectType = ELogisticsObjectType::Facility;
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
	float OffsetX = LogisticsInputPortArr[0]->GetScaledBoxExtent().X + 1.f;
	PortBlockWarningComponent->AddRelativeLocation(FVector(-OffsetX, 0.f, 0.f)); // 설비에 묻히지 않게 조금 이동
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
			InputPort->PendingItem = FFactoryItemInstance();
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
	UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem)
{
	if (!RequestPort || !IncomingItem) return false;
	
	return RequestPort->PendingItem.ItemData == nullptr;
}

