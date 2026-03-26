// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/FactoryInteractionPoint.h"

#include "Components/WidgetComponent.h"
#include "Player/Component/FactoryInventoryComponent.h"
#include "Items/FactoryItemData.h"
#include "Player/Component/FactoryPlacementComponent.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"


AFactoryInteractionPoint::AFactoryInteractionPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
	
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	WidgetComponent->SetDrawSize(FVector2D(400.0f, 100.0f));
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 50.0f));
}

void AFactoryInteractionPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (WidgetComponent)
	{
		if (!WidgetComponent->GetUserWidgetObject())
		{
			WidgetComponent->InitWidget();
		}
		TArray<FInteractionOption> Options;
		if (TryGetInteractionOptions(EPlacementMode::None, Options))
		{
			UpdateWidgetText(Options[0].DisplayText);
		}
	}
}

void AFactoryInteractionPoint::BeginPlay()
{
	Super::BeginPlay();
	if (WidgetComponent)
	{
		TArray<FInteractionOption> Options;
		if (TryGetInteractionOptions(EPlacementMode::None, Options) && Options.Num() > 0)
		{
			UpdateWidgetText(Options[0].DisplayText);
		}
	}
}

void AFactoryInteractionPoint::Interact(const AActor* Interactor, const EPlacementMode CurrentMode, int32 OptionIndex)
{
	if (!ItemData || !Interactor || Amount <= 0) return;
	if (CurrentMode != EPlacementMode::None) return;
	
	UFactoryInventoryComponent* Inventory = Cast<APawn>(Interactor)->GetController()->FindComponentByClass<UFactoryInventoryComponent>();
	UFactoryWarehouseSubsystem* Warehouse = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
	
	switch (PointType)
	{
	case EInteractionPointType::GiveToInventory:
		if (Inventory)
		{
			int32 Added = Inventory->AutoAddItem(ItemData.Get(), Amount);
			UE_LOG(LogTemp, Log, TEXT("Inventory : Added %d %s"), Added, *ItemData->ItemName.ToString());
		}
		break;
	case EInteractionPointType::TakeToInventory:
		if (Inventory)
		{
			if (Inventory->AutoRemoveItem(ItemData.Get(), Amount))
			{
				UE_LOG(LogTemp, Log, TEXT("Inventory : Removed %d %s"), Amount, *ItemData->ItemName.ToString());
			}
		}
		break;
	case EInteractionPointType::GiveToWarehouse:
		if (Warehouse)
		{
			int32 Added = Warehouse->AddItem(ItemData, Amount);
			UE_LOG(LogTemp, Log, TEXT("Warehouse : Added %d %s"), Added, *ItemData->ItemName.ToString());
		}
		break;
	case EInteractionPointType::TakeToWarehouse:
		if (Warehouse)
		{
			if (Warehouse->TryRemoveItem(ItemData.Get(), Amount))
			{
				UE_LOG(LogTemp, Log, TEXT("Warehouse : Removed %d %s"), Amount, *ItemData->ItemName.ToString());
			}
		}
		break;
	}
}

bool AFactoryInteractionPoint::TryGetInteractionOptions(const EPlacementMode CurrentMode, TArray<FInteractionOption>& OutOptions) const
{
	if (CurrentMode != EPlacementMode::None) return false;
	
	FText ResultText;
	
	if (!InteractText.IsEmpty())
	{
		ResultText = InteractText;
	}
	else
	{
		if (!ItemData) return false;
		FString TargetStr, ModeStr;
		switch (PointType)
		{
		case EInteractionPointType::GiveToInventory:
			TargetStr = TEXT("인벤토리");
			ModeStr = TEXT("증가");
			break;
		case EInteractionPointType::TakeToInventory:
			TargetStr = TEXT("인벤토리");
			ModeStr = TEXT("감소");
			break;
		case EInteractionPointType::GiveToWarehouse:
			TargetStr = TEXT("창고");
			ModeStr = TEXT("증가");
			break;
		case EInteractionPointType::TakeToWarehouse:
			TargetStr = TEXT("창고");
			ModeStr = TEXT("감소");
			break;
		}
		ResultText = FText::FromString(FString::Printf(
			TEXT("%s에 %s %d개 %s"), *TargetStr, *ItemData->ItemName.ToString(), Amount, *ModeStr));
	}
	
	FInteractionOption Option;
	Option.OptionID = TEXT("DefaultInteractPoint");
	Option.DisplayText = ResultText;
	OutOptions.Add(Option);
	
	return true;
}


