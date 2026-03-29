// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryWarehouseExporter.h"

#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
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
	
	LogisticsObjectType = ELogisticsObjectType::Facility;
}

void AFactoryWarehouseExporter::BeginPlay()
{
	Super::BeginPlay();
	
	OutputPortBlockedStates.Init(false, LogisticsOutputPortArr.Num());
	OutputPortPulledThisCycle.Init(false, LogisticsOutputPortArr.Num());
	
	if (RecipeBillboardComponent)
	{
		OnTargetItemChanged.AddDynamic(RecipeBillboardComponent, &UFactoryRecipeBillboardComponent::OnItemChangedCallback);
		RecipeBillboardComponent->OnItemChangedCallback(TargetItemData);
	}
	
	if (LogisticsOutputPortArr.IsValidIndex(0) && LogisticsOutputPortArr[0])
	{
		UFactoryPortBlockWarningComponent* PortBlockWarningComponent = NewObject<UFactoryPortBlockWarningComponent>(this);
		PortBlockWarningComponent->SetupAttachment(LogisticsOutputPortArr[0]);
		float OffsetX = LogisticsOutputPortArr[0]->GetScaledBoxExtent().X + 1.f;
		PortBlockWarningComponent->AddRelativeLocation(FVector(OffsetX, 0.f, 0.f)); // 설비에 묻히지 않게 조금 이동
		PortBlockWarningComponent->RegisterComponent();
		LogisticsOutputPortArr[0]->OnPortBlockedStateChanged.AddDynamic(PortBlockWarningComponent, &UFactoryPortBlockWarningComponent::OnPortBlockedCallback);
	}
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
	for (int i = 0; i < OutputPortPulledThisCycle.Num(); i++)
	{
		OutputPortBlockedStates[i] = false;
		OutputPortPulledThisCycle[i] = false;
	}
	TryPushOutputToPorts();
}

void AFactoryWarehouseExporter::LatePlanCycle()
{
	TryPushOutputToPorts();
}

void AFactoryWarehouseExporter::ExecuteCycle()
{

}

void AFactoryWarehouseExporter::UpdateView()
{
	UFactoryWarehouseSubsystem* Warehouse = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
	
	bool bIsOutOfStock = TargetItemData && (!Warehouse || Warehouse->GetItemAmount(TargetItemData) <= 0);
	
	for (int i = 0; i < LogisticsOutputPortArr.Num(); i++)
	{
		if (LogisticsOutputPortArr[i])
		{
			bool bIsBlocked = bIsOutOfStock || OutputPortBlockedStates[i];
			LogisticsOutputPortArr[i]->SetPortBlocked(bIsBlocked);
		}
	}
}

const UFactoryItemData* AFactoryWarehouseExporter::PeekOutputItem(UFactoryOutputPortComponent* RequestPort)
{
	return nullptr;
}

FFactoryItemInstance AFactoryWarehouseExporter::ConsumeItem(UFactoryOutputPortComponent* RequestPort)
{
	return FFactoryItemInstance();
}

bool AFactoryWarehouseExporter::CanReceiveItem(UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem)
{
	// Exporter에서는 사용되지 않음
	return false;
}

void AFactoryWarehouseExporter::ReceiveItem(UFactoryInputPortComponent* RequestPort, FFactoryItemInstance Item)
{
	
}

void AFactoryWarehouseExporter::TryPushOutputToPorts()
{
	// TODO : WarehouseExporter가 라운드 로빈 방식으로 작동하게끔 조정필요
	if (!TargetItemData) return;
	
	UFactoryWarehouseSubsystem* Warehouse = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
	if (!Warehouse || Warehouse->GetItemAmount(TargetItemData) <= 0) return; // 창고 비었으면 퇴근
	
	int32 MaxOutputs = LogisticsOutputPortArr.Num();
	if (MaxOutputs <= 0) return;
	
	int32 StartIndex = OutputPortIndex;
	for (int i = 0; i < MaxOutputs; i++)
	{
		// 창고 수량이 바닥나면 즉시 중단
		if (Warehouse->GetItemAmount(TargetItemData) <= 0) break; 
		
		int32 Index = (StartIndex + i) % MaxOutputs;
		if (OutputPortPulledThisCycle[Index]) continue;
		
		UFactoryOutputPortComponent* OutPort = LogisticsOutputPortArr[Index];
		if (!OutPort) continue;
		
		UFactoryInputPortComponent* TargetIn = OutPort->GetConnectedInput();
		if (!TargetIn) continue;
		
		AFactoryLogisticsObjectBase* TargetObj = TargetIn->GetPortOwner();
		if (!TargetObj) continue;
		
		if (TargetObj->CanReceiveItem(TargetIn, TargetItemData))
		{
			// 타겟이 받을 수 있다면 창고에서 차감 시도
			if (Warehouse->TryRemoveItem(TargetItemData, 1))
			{
				FFactoryItemInstance PushedItem(TargetItemData);
				TargetObj->ForceAcceptPushedItem(TargetIn, PushedItem); 
				
				OutputPortPulledThisCycle[Index] = true; 
				OutputPortBlockedStates[Index] = false;
				OutputPortIndex = (Index + 1) % MaxOutputs;
				
				int32 WarehouseAmount = Warehouse->GetItemAmount(TargetItemData);
				OnWarehouseAmountUpdated.Broadcast(WarehouseAmount); 
			}
		}
		else
		{
			OutputPortBlockedStates[Index] = true;
		}
	}
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
