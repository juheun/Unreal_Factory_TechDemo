// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/FactoryItemVisual.h"
#include "Items/FactoryItemData.h"


// Sets default values
AFactoryItemVisual::AFactoryItemVisual()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AFactoryItemVisual::UpdateVisual(const class UFactoryItemData* ItemData)
{
	if (!ItemData || ItemData->ItemICon) return;
	
	if (!DynamicMaterial)
	{
		DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0);
	}
	
	if (DynamicMaterial)
	{
		DynamicMaterial->SetTextureParameterValue(FName("ItemIcon"), ItemData->ItemICon);
	}
}

