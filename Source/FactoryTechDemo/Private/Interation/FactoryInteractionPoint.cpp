// Fill out your copyright notice in the Description page of Project Settings.


#include "Interation/FactoryInteractionPoint.h"

#include "Inventory/FactoryInventoryComponent.h"
#include "Player/Component/FactoryPlacementComponent.h"


// Sets default values
AFactoryInteractionPoint::AFactoryInteractionPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
}

void AFactoryInteractionPoint::Interact(const AActor* Interactor, const EPlacementMode CurrentMode)
{
	if (!ItemToGive || !Interactor || AmountToGive <= 0) return;
	
	if (CurrentMode != EPlacementMode::None) return;
	
	if (UFactoryInventoryComponent* Inventory = 
		Cast<APawn>(Interactor)->GetController()->FindComponentByClass<UFactoryInventoryComponent>())
	{
		int32 Added = Inventory->AutoAddItem(ItemToGive.Get(), AmountToGive);
		UE_LOG(LogTemp, Log, TEXT("Added %d items"), Added);
	}
}

bool AFactoryInteractionPoint::TryGetInteractText(const EPlacementMode CurrentMode, FText& OutText) const
{
	if (CurrentMode == EPlacementMode::None)
	{
		OutText = InteractText;
		return true;
	}
	
	return false;
}


