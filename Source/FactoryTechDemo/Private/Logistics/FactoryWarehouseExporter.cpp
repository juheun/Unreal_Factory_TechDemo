// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryWarehouseExporter.h"

#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"
#include "UI/World/FactoryPortBlockWarningComponent.h"
#include "UI/World/FactoryRecipeBillboardComponent.h"
#include "UI/World/FactorySmartNameplateComponent.h"


AFactoryWarehouseExporter::AFactoryWarehouseExporter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SmartNameplateComponent = CreateDefaultSubobject<UFactorySmartNameplateComponent>(TEXT("SartNameplateComponent"));
	SmartNameplateComponent->SetupAttachment(RootComponent);
	
	RecipeBillboardComponent = CreateDefaultSubobject<UFactoryRecipeBillboardComponent>(TEXT("RecipeBillboardComponent"));
	RecipeBillboardComponent->SetupAttachment(RootComponent);
	
	FacilityMenuMode = EFactoryMenuMode::FacilityOnly;
}

void AFactoryWarehouseExporter::BeginPlay()
{
	Super::BeginPlay();
	
	if (RecipeBillboardComponent)
	{
		OnTargetItemChanged.AddDynamic(RecipeBillboardComponent, &UFactoryRecipeBillboardComponent::OnItemChangedCallback);
		RecipeBillboardComponent->OnItemChangedCallback(TargetItemData);
	}
	
	UFactoryPortBlockWarningComponent* PortBlockWarningComponent = NewObject<UFactoryPortBlockWarningComponent>(this);
	PortBlockWarningComponent->SetupAttachment(LogisticsOutputPortArr[0]);
	float OffsetX = LogisticsOutputPortArr[0]->GetScaledBoxExtent().X + 1.f;
	PortBlockWarningComponent->AddRelativeLocation(FVector(OffsetX, 0.f, 0.f)); // 설비에 묻히지 않게 조금 이동
	PortBlockWarningComponent->RegisterComponent();
	LogisticsOutputPortArr[0]->OnPortBlockedStateChanged.AddDynamic(PortBlockWarningComponent, &UFactoryPortBlockWarningComponent::OnPortBlockedCallback);
}

void AFactoryWarehouseExporter::InitObject(const UFactoryObjectData* Data)
{
	Super::InitObject(Data);
	
	// 설비 이름 표시
	if (SmartNameplateComponent)
	{
		SmartNameplateComponent->InitNameplate(Data);
	}
}

void AFactoryWarehouseExporter::PlanCycle()
{
	// TODO : 현재 방식은 창고에 아이템이 적고, Exporter가 많으면 특정 Exporter만 자원을 독식 할 수 있음.
	// 추후 라운드 로빈식으로 모든 Exporter를 통제하는 방식으로 수정 필요
	
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
				TargetPort->PendingItem = NewInstance;	// 상대방 Input에 아이템 밀어넣기
				LogisticsOutputPortArr[0]->SetPortBlocked(false);
			}
			else
			{
				LogisticsOutputPortArr[0]->SetPortBlocked(true);
			}
		}
		else
		{
			LogisticsOutputPortArr[0]->SetPortBlocked(true);
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

bool AFactoryWarehouseExporter::CanPushItemFromBeforeObject(UFactoryInputPortComponent* RequestPort,
	const UFactoryItemData* IncomingItem)
{
	return false;
}

bool AFactoryWarehouseExporter::PullItemFromInputPorts(FFactoryItemInstance& Item)
{
	return false;
}

void AFactoryWarehouseExporter::SetTargetItem(const UFactoryItemData* NewTargetItem)
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
