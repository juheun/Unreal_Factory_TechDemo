// Fill out your copyright notice in the Description page of Project Settings.


#include "Placement/FactoryPlaceObjectBase.h"

#include "Player/Component/FactoryInventoryComponent.h"
#include "Items/FactoryFacilityItemData.h"
#include "Placement/FactoryObjectData.h"
#include "Player/FactoryPlayerController.h"
#include "Player/Component/FactoryPlacementComponent.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"
#include "UI/Facility/FactoryFacilityPanelBase.h"

AFactoryPlaceObjectBase::AFactoryPlaceObjectBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	RootComponent = MeshComponent;
	MeshComponent->bReceivesDecals = false;
	
	MeshComponent->SetCollisionObjectType(ECC_GameTraceChannel3);
	MeshComponent->SetGenerateOverlapEvents(true);
}

void AFactoryPlaceObjectBase::BeginPlay()
{
	Super::BeginPlay();
	
	MeshComponent->SetCollisionObjectType(ECC_GameTraceChannel3);
	MeshComponent->SetGenerateOverlapEvents(true);
}

void AFactoryPlaceObjectBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	// bRefundItemOnDestroy가 false면 반환이 필요없기에 생략
	if (PlacementDataAsset->RepresentingItemData && PlacementDataAsset->bRefundItemOnDestroy)
	{
		if (AFactoryPlayerController* PlayerController = Cast<AFactoryPlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			if (PlayerController->GetInventoryComponent()->AutoAddItem(PlacementDataAsset->RepresentingItemData, 1) > 0)
			{
				return;
			}
		}
	
		UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
		if (!WarehouseSubsystem) return;
	
		WarehouseSubsystem->AddItem(PlacementDataAsset->RepresentingItemData, 1);
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
		if (PlacementDataAsset && PlacementDataAsset->FacilityPanelBP)
		{
			if (const APawn* InteractorPawn = Cast<APawn>(Interactor))
			{
				if (AFactoryPlayerController* PC = Cast<AFactoryPlayerController>(InteractorPawn->GetController()))
				{
					PC->OpenFacilityMenu(this, FacilityMenuMode);
				}
			}
		}
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
		if (PlacementDataAsset->FacilityPanelBP)
		{
			OutText = FText::Format(FText::FromString(TEXT("{0} 패널 열기")), Name);
			return true;
		}
		return false;
	}
	
	return false;
}

void AFactoryPlaceObjectBase::Retrieve()
{
	// TODO : 수납 애니메이션, 연출 등
	
	Destroy();
}

