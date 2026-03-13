// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/FactoryQuickSlotComponent.h"

#include "EnhancedInputComponent.h"
#include "Items/FactoryFacilityItemData.h"
#include "Placement/FactoryObjectData.h"
#include "Player/FactoryPlayerController.h"
#include "Player/Component/FactoryInventoryComponent.h"
#include "Player/Input/FactoryInputConfig.h"
#include "UI/QuickSlot/FactoryQuickBarWidget.h"


UFactoryQuickSlotComponent::UFactoryQuickSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFactoryQuickSlotComponent::SetUpInputComponent(UEnhancedInputComponent* PlayerInputComp,
	const UFactoryInputConfig* InputConfig)
{
	for (int i = 0; i < InputConfig->QuickSlotActionArr.Num(); i++)
	{
		if (InputConfig->QuickSlotActionArr[i])
		{
			PlayerInputComp->BindAction(InputConfig->QuickSlotActionArr[i], ETriggerEvent::Started, this, &UFactoryQuickSlotComponent::ExecuteQuickSlotData, i);
		}
	}
}

void UFactoryQuickSlotComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (QuickSlotObjectDataArr.Num() < 10)
	{
		QuickSlotObjectDataArr.SetNum(10);
	}
	
	CachedPlayerController = Cast<AFactoryPlayerController>(GetOwner());
	if (AFactoryPlayerController* Controller = CachedPlayerController.Get())
	{
		CachedInventory = Controller->GetInventoryComponent();
		if (CachedInventory.IsValid())
		{
			CachedInventory->OnSlotUpdated.AddDynamic(this, &UFactoryQuickSlotComponent::HandleInventoryUpdated);
		}
		
		if (Controller->IsLocalController() && QuickBarWidgetBP)
		{
			QuickBarWidget = CreateWidget<UFactoryQuickBarWidget>(Controller, QuickBarWidgetBP);
			if (QuickBarWidget)
			{
				QuickBarWidget->AddToViewport();
				QuickBarWidget->InitQuickBar(this);
			}
		}
	}
}

void UFactoryQuickSlotComponent::SetQuickSlotData(int32 Index, UFactoryObjectData* InData)
{
	if (!QuickSlotObjectDataArr.IsValidIndex(Index)) return;
	
	if (InData != nullptr)	// 만약 이미 퀵슬롯에 같은 데이터가 등록되어 있었다면, 이동으로 처리
	{
		for (int32 i = 0; i < QuickSlotObjectDataArr.Num(); i++)
		{
			if (i != Index && QuickSlotObjectDataArr[i] == InData)
			{
				QuickSlotObjectDataArr[i] = nullptr;
				BroadcastQuickSlotChange(i);
				break;
			}
		}
	}
	QuickSlotObjectDataArr[Index] = InData;
	BroadcastQuickSlotChange(Index);
}

void UFactoryQuickSlotComponent::SwapQuickSlotData(int32 IndexA, int32 IndexB)
{
	if (!QuickSlotObjectDataArr.IsValidIndex(IndexA) || !QuickSlotObjectDataArr.IsValidIndex(IndexB) || IndexA == IndexB) return;

	UFactoryObjectData* Temp = QuickSlotObjectDataArr[IndexA];
	QuickSlotObjectDataArr[IndexA] = QuickSlotObjectDataArr[IndexB];
	QuickSlotObjectDataArr[IndexB] = Temp;

	BroadcastQuickSlotChange(IndexA);
	BroadcastQuickSlotChange(IndexB);
}

void UFactoryQuickSlotComponent::BroadcastQuickSlotChange(int32 Index)
{
	if (!QuickSlotObjectDataArr.IsValidIndex(Index)) return;

	UFactoryObjectData* Data = QuickSlotObjectDataArr[Index];
	int32 TotalAmount = 0;

	// 인벤토리에서 실제 갯수 계산
	if (Data && Data->RepresentingItemData && CachedInventory.IsValid())
	{
		TotalAmount = CachedInventory->GetTotalItemAmount(Data->RepresentingItemData);
	}

	OnQuickSlotDataChanged.Broadcast(Index, Data, TotalAmount);
}

void UFactoryQuickSlotComponent::HandleInventoryUpdated(int32 SlotIndex, FFactorySlot SlotData)
{
	for (int32 i = 0; i < QuickSlotObjectDataArr.Num(); ++i)
	{
		if (QuickSlotObjectDataArr[i] != nullptr)
		{
			BroadcastQuickSlotChange(i);
		}
	}
}

void UFactoryQuickSlotComponent::ExecuteQuickSlotData(int Index)
{
	UFactoryObjectData* ObjectData = QuickSlotObjectDataArr.IsValidIndex(Index) ? QuickSlotObjectDataArr[Index] : nullptr;
	if (ObjectData)
	{
		OnQuickSlotExecuted.Broadcast(ObjectData);
	}
}




