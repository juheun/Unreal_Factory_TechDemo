// Fill out your copyright notice in the Description page of Project Settings.


#include "Placement/FactoryPlaceObjectBase.h"
#include "Placement/FactoryObjectData.h"

// Sets default values
AFactoryPlaceObjectBase::AFactoryPlaceObjectBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	RootComponent = MeshComponent;
	MeshComponent->bReceivesDecals = false;
}

void AFactoryPlaceObjectBase::InitObject(const UFactoryObjectData* Data)
{
	if (!Data) return;
	DataAsset = Data;
}

// Called when the game starts or when spawned
void AFactoryPlaceObjectBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFactoryPlaceObjectBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

