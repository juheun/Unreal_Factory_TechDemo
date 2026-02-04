// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryPlacePreview.h"
#include "FactoryObjectData.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"

// Sets default values
AFactoryPlacePreview::AFactoryPlacePreview()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	GridDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("GridDecal"));
	GridDecalComponent->SetupAttachment(RootComponent);
	GridDecalComponent->SetRelativeLocation(FVector(0.f, 0.f, 50.f));	// 데칼 적용을 위해 일부 들어올림
	GridDecalComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	
	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->SetupAttachment(RootComponent);
	OverlapBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AFactoryPlacePreview::InitPreview(const UFactoryObjectData* Data)
{
	if (!Data) return;
	
	ObjectData = Data;
	
	if (Data->ObjectMesh)
	{
		MeshComponent->SetStaticMesh(Data->ObjectMesh);
	}
	
	PreviewDynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0);
	
	//TODO: 매직넘버 처리
	float BoxX = Data->GridSize.X * 100.f * 0.5f;
	float BoxY = Data->GridSize.Y * 100.f * 0.5f;
	OverlapBox->SetBoxExtent(FVector(BoxX, BoxY, 50.f));
	
	float DecalRangeX = BoxX + 500.f;
	float DecalRangeY = BoxY + 500.f;
	GridDecalComponent->DecalSize = FVector(200.f, DecalRangeX, DecalRangeY);	// 투사깊이, x, y
	
	GridDynamicMaterial = GridDecalComponent->CreateDynamicMaterialInstance();
}

void AFactoryPlacePreview::SetPlacementValid(bool bIsValid)
{
	bIsPlacementValid = bIsValid;
	// TODO : 배치가능 상태 변경시 처리
}

// Called when the game starts or when spawned
void AFactoryPlacePreview::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFactoryPlacePreview::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

