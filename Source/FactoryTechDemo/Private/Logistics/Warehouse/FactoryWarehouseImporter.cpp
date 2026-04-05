// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/Warehouse/FactoryWarehouseImporter.h"

#include "Logistics/Ports/FactoryInputPortComponent.h"
#include "Logistics/Ports/FactoryOutputPortComponent.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"
#include "UI/World/Components/FactoryPortBlockWarningComponent.h"
#include "UI/World/Components/FactoryRecipeBillboardComponent.h"
#include "UI/World/Components/FactorySmartNameplateComponent.h"


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
	if (PlacementDataAsset && SmartNameplateComponent)
	{
		SmartNameplateComponent->InitNameplate(PlacementDataAsset);
	}
	
	if (LogisticsInputPortArr.IsValidIndex(0) && LogisticsInputPortArr[0])
	{
		UFactoryPortBlockWarningComponent* PortBlockWarningComponent = NewObject<UFactoryPortBlockWarningComponent>(this);
		PortBlockWarningComponent->SetupAttachment(LogisticsInputPortArr[0]);
		float OffsetX = LogisticsInputPortArr[0]->GetScaledBoxExtent().X + 1.f;
		PortBlockWarningComponent->AddRelativeLocation(FVector(-OffsetX, 0.f, 0.f)); // 설비에 묻히지 않게 조금 이동
		PortBlockWarningComponent->RegisterComponent();
		LogisticsInputPortArr[0]->OnPortBlockedStateChanged.AddDynamic(PortBlockWarningComponent, &UFactoryPortBlockWarningComponent::OnPortBlockedCallback);
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

void AFactoryWarehouseImporter::InitPhase()
{
}

void AFactoryWarehouseImporter::LogisticsPhase()
{
	TryPullInputFromPorts();
}

void AFactoryWarehouseImporter::LateLogisticsPhase()
{
	TryPullInputFromPorts();
}

void AFactoryWarehouseImporter::LogicPhase()
{

}

void AFactoryWarehouseImporter::VisualPhase()
{
	if (LogisticsInputPortArr.IsValidIndex(0) && LogisticsInputPortArr[0])
	{
		LogisticsInputPortArr[0]->SetPortBlocked(bIsInputBlocked);
	}
}

void AFactoryWarehouseImporter::TryPullInputFromPorts()
{
	if (!LogisticsInputPortArr.IsValidIndex(0) || !LogisticsInputPortArr[0]) return;
	
	UFactoryInputPortComponent* InputPort = LogisticsInputPortArr[0];
	UFactoryOutputPortComponent* ConnectedOut = InputPort->GetConnectedOutput();
	
	if (!ConnectedOut) 
	{
		bIsInputBlocked = false; // 연결 안 됐으면 막힌 게 아님
		return;
	}
	
	AFactoryLogisticsObjectBase* PrevObj = ConnectedOut->GetPortOwner();
	if (!PrevObj) return;
	
	const UFactoryItemData* PeekedItem = PrevObj->PeekOutputItem(ConnectedOut);
	if (PeekedItem)
	{
		if (CanReceiveItem(InputPort, PeekedItem))
		{
			FFactoryItemInstance PulledItem = PrevObj->ConsumeItem(ConnectedOut);
			ReceiveItem(InputPort, PulledItem);
			bIsInputBlocked = false;
		}
		else
		{
			bIsInputBlocked = true; 
		}
	}
	else
	{
		bIsInputBlocked = false; // 줄 게 없어서 대기 중일 뿐, 막힌 건 아님
	}
}

void AFactoryWarehouseImporter::ProcessImport(FFactoryItemInstance Item)
{
	if (!Item.IsValid()) return;

	if (UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>())
	{
		int ItemAmountBeforeAdd = WarehouseSubsystem->GetItemAmount(Item.ItemData);
		int ItemAmountAfterAdd = WarehouseSubsystem->AddItem(Item.ItemData, 1);
		
		if (ItemAmountBeforeAdd < ItemAmountAfterAdd)
		{
			// UI 갱신 로직
			if (!CachedLastImportedItem || CachedLastImportedItem != Item.ItemData)
			{
				CachedLastImportedItem = Item.ItemData;
				OnImportItemChanged.Broadcast(Item.ItemData);
			}
			bIsInputBlocked = false;
		}
		else
		{
			// AddItem이 실패했다 = 창고 용량이 꽉 찼다
			bIsInputBlocked = true;
		}
	}
}

bool AFactoryWarehouseImporter::CanReceiveItem(UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem)
{
	if (!IncomingItem) return false;
	
	if (UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>())
	{
		return WarehouseSubsystem->GetItemAmount(IncomingItem) < WarehouseSubsystem->GetMaxItemAmount();
	}
	return false;
}

void AFactoryWarehouseImporter::ReceiveItem(UFactoryInputPortComponent* RequestPort, FFactoryItemInstance Item)
{
	ProcessImport(Item);
}

