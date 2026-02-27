// Fill out your copyright notice in the Description page of Project Settings.


#include "Interation/FactoryInteractionPoint.h"

#include "Inventory/FactoryInventoryComponent.h"
#include "Items/FactoryItemData.h"


// Sets default values
AFactoryInteractionPoint::AFactoryInteractionPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
}

void AFactoryInteractionPoint::Interact(AActor* Interactor)
{
	if (!ItemToGive || !Interactor || AmountToGive <= 0) return;
	
	if (UFactoryInventoryComponent* Inventory = 
		Cast<APawn>(Interactor)->GetController()->FindComponentByClass<UFactoryInventoryComponent>())
	{
		int32 Added = Inventory->AutoAddItem(ItemToGive.Get(), AmountToGive);
		UE_LOG(LogTemp, Log, TEXT("Added %d items"), Added);
	}
}

FText AFactoryInteractionPoint::GetInteractText() const
{
	return InteractText;
}


