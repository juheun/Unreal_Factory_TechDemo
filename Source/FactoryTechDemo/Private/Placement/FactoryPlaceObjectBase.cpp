// Fill out your copyright notice in the Description page of Project Settings.


#include "Placement/FactoryPlaceObjectBase.h"

#include "Inventory/FactoryInventoryComponent.h"
#include "Items/FactoryFacilityItemData.h"
#include "Placement/FactoryObjectData.h"
#include "Player/FactoryPlayerController.h"
#include "Player/Component/FactoryPlacementComponent.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"

AFactoryPlaceObjectBase::AFactoryPlaceObjectBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	RootComponent = MeshComponent;
	MeshComponent->bReceivesDecals = false;
}

void AFactoryPlaceObjectBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (PlacementDataAsset->ItemData)	// ItemData는 무한으로 제공되는 설비 (예: 벨트 등)은 반환이 필요 없기에 일부러 비워둠
	{
		if (AFactoryPlayerController* PlayerController = Cast<AFactoryPlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			if (PlayerController->GetInventoryComponent()->AutoAddItem(PlacementDataAsset->ItemData, 1) > 0)
			{
				return;
			}
		}
	
		UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
		if (!WarehouseSubsystem) return;
	
		WarehouseSubsystem->StoreItem(PlacementDataAsset->ItemData, 1);
	}
}

void AFactoryPlaceObjectBase::InitObject(const UFactoryObjectData* Data)
{
	if (!Data) return;
	PlacementDataAsset = Data;
}

void AFactoryPlaceObjectBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFactoryPlaceObjectBase::Interact(const AActor* Interactor, const EPlacementMode CurrentMode)
{
	if (CurrentMode == EPlacementMode::Retrieve)
	{
		Retrieve();
	}
	else if (CurrentMode == EPlacementMode::None)
	{
		// TODO : 패널로직
	}
}

bool AFactoryPlaceObjectBase::TryGetInteractText(const EPlacementMode CurrentMode, FText& OutText) const
{
	if (!PlacementDataAsset) return false;
	FText Name = PlacementDataAsset->ObjectName;
	if (CurrentMode == EPlacementMode::Retrieve)
	{
		OutText = FText::Format(FText::FromString(TEXT("{0} 수납")), Name);
		return true;
	}
	else if (CurrentMode == EPlacementMode::None)
	{
		OutText = FText::Format(FText::FromString(TEXT("{0} 패널 열기")), Name);
		return true;
	}
	
	return false;
}

void AFactoryPlaceObjectBase::Retrieve()
{
	// TODO : 수납 애니메이션, 연출 등
	
	Destroy();
}

