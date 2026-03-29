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
	
	LogisticsObjectType = ELogisticsObjectType::Conveyor;
}

void AFactoryBeltRouter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
	if (!WarehouseSubsystem) return;

	if (CurrentItem.IsValid())
	{
		WarehouseSubsystem->AddItem(CurrentItem.ItemData.Get(), 1);
	}
}

void AFactoryBeltRouter::PlanCycle()
{
	TryPushOutputToPorts();
	TryPullInputFromPorts();
}

void AFactoryBeltRouter::LatePlanCycle()
{
	TryPushOutputToPorts();
	TryPullInputFromPorts();
}

void AFactoryBeltRouter::ExecuteCycle()
{
	bReceivedThisCycle = false;
}

void AFactoryBeltRouter::UpdateView()
{
	// 애니메이션 재생 등
}

const UFactoryItemData* AFactoryBeltRouter::PeekOutputItem(UFactoryOutputPortComponent* RequestPort)
{
	if (bReceivedThisCycle) return nullptr;
	if (!CurrentItem.IsValid()) return nullptr;
	
	int32 Index = LogisticsOutputPortArr.IndexOfByKey(RequestPort);
	
	// 내가 정해둔 차례의 포트에게만 아이템을 보여줌
	if (Index == CurrentOutputIndex) return CurrentItem.ItemData;
	
	return nullptr;
}

FFactoryItemInstance AFactoryBeltRouter::ConsumeItem(UFactoryOutputPortComponent* RequestPort)
{
	if (bReceivedThisCycle) return FFactoryItemInstance();
	if (!CurrentItem.IsValid()) return FFactoryItemInstance();
	
	int32 Index = LogisticsOutputPortArr.IndexOfByKey(RequestPort);
	
	if (Index == CurrentOutputIndex)
	{
		FFactoryItemInstance GivenItem = CurrentItem;
		CurrentItem = FFactoryItemInstance();
		CurrentOutputIndex = (CurrentOutputIndex + 1) % LogisticsOutputPortArr.Num();
		return GivenItem;
	}
	
	return FFactoryItemInstance();
}

bool AFactoryBeltRouter::CanReceiveItem(UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem)
{
	if (!IncomingItem) return false;
	if (CurrentItem.ItemData) return false;	// 이미 아이템이 있으면 받을 수 없음 
	return true;
}

void AFactoryBeltRouter::ReceiveItem(UFactoryInputPortComponent* RequestPort, FFactoryItemInstance Item)
{
	if (!Item.IsValid()) return;
	CurrentItem = Item;
	
	bReceivedThisCycle = true;
}

void AFactoryBeltRouter::TryPushOutputToPorts()
{
	if (!CurrentItem.IsValid()) return;
	
	int32 MaxOutputs = LogisticsOutputPortArr.Num();
	if (MaxOutputs <= 0) return;
	
	int32 StartIndex = CurrentOutputIndex;
	for (int i = 0; i < MaxOutputs; i++)
	{
		int32 Index = (StartIndex + i) % MaxOutputs;
		UFactoryOutputPortComponent* OutPort = LogisticsOutputPortArr[Index];
		if (!OutPort) continue;
		
		UFactoryInputPortComponent* TargetIn = OutPort->GetConnectedInput();
		if (!TargetIn) continue;
		
		AFactoryLogisticsObjectBase* TargetObj = TargetIn->GetPortOwner();
		if (!TargetObj) continue;
		
		// 막힌 곳은 가볍게 Skip하고 뚫린 곳에 즉시 넣음
		if (TargetObj->CanReceiveItem(TargetIn, CurrentItem.ItemData))
		{
			TargetObj->ForceAcceptPushedItem(TargetIn, CurrentItem); // 상대방 버퍼로 전송 완료!
			CurrentItem = FFactoryItemInstance(); // 내 주머니 비우기
			CurrentOutputIndex = (Index + 1) % MaxOutputs; // 라운드 로빈 갱신
			break;
		}
	}
}

void AFactoryBeltRouter::TryPullInputFromPorts()
{
	if (CurrentItem.ItemData) return;
	
	int32 MaxInputs = LogisticsInputPortArr.Num();
	if (MaxInputs <= 0) return;
	
	int32 StartIndex = CurrentInputIndex;
	for (int i = 0; i < MaxInputs; i++)
	{
		int Index = (StartIndex + i) % MaxInputs;
		
		UFactoryInputPortComponent* InputPort = LogisticsInputPortArr[Index];
		if (!InputPort) continue;
		UFactoryOutputPortComponent* ConnectedOut = InputPort->GetConnectedOutput();
		if (!ConnectedOut) continue;
		AFactoryLogisticsObjectBase* PrevObj = ConnectedOut->GetPortOwner();
		if (!PrevObj) continue;
		
		const UFactoryItemData* PeekedItem = PrevObj->PeekOutputItem(ConnectedOut);
		if (PeekedItem && CanReceiveItem(InputPort, PeekedItem))
		{
			FFactoryItemInstance PulledItem = PrevObj->ConsumeItem(ConnectedOut);
			ReceiveItem(InputPort, PulledItem);
			CurrentInputIndex = (Index + 1) % MaxInputs;	// 라운드 로빈 시작 다음차례로
			break;
		}
	}
}
