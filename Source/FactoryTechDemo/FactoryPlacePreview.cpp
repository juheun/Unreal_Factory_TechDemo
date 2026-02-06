// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryPlacePreview.h"

#include "FactoryBuildingSettings.h"
#include "FactoryObjectData.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "FactoryPlaceObjectBase.h"

// Sets default values
AFactoryPlacePreview::AFactoryPlacePreview()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetReceivesDecals(false);
	
	GridDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("GridDecal"));
	GridDecalComponent->SetupAttachment(RootComponent);
	GridDecalComponent->SetRelativeLocation(FVector(0.f, 0.f, 1.f));	// 데칼 적용을 위해 살짝 들어올림
	GridDecalComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	ConstructorHelpers::FObjectFinder<UMaterialInterface> GridMatAsset(TEXT("/Game/Material/M_BuildGrid.M_BuildGrid"));
	if (GridMatAsset.Succeeded())
	{
		GridDecalComponent->SetDecalMaterial(GridMatAsset.Object);
	}
	
	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->SetupAttachment(RootComponent);
	OverlapBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AFactoryPlacePreview::InitPreview(const UFactoryObjectData* Data)
{
	if (!Data) return;
	
	ObjectData = Data;
	
	if (Data->PlaceObjectBP)
	{
		AFactoryPlaceObjectBase* PlaceObjectBase = Data->PlaceObjectBP->GetDefaultObject<AFactoryPlaceObjectBase>();
		MeshComponent->SetStaticMesh(PlaceObjectBase->GetStaticMesh());
	}
	
	const UFactoryBuildingSettings* BuildingSettings = GetDefault<UFactoryBuildingSettings>();
	if (!BuildingSettings) return;
	
	UMaterialInterface* PreviewMaterial = BuildingSettings->GetPlacePreviewMaterial();
	
	MeshComponent->SetMaterial(0, PreviewMaterial);
	PreviewDynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0);
	
	float GridLength = BuildingSettings->GetGridLength();
	
	float BoxX = Data->GridSize.X * GridLength * 0.5f - 1.f;	// 바로 옆 그리드와 붙는것 방지하기 위해 1 빼줌
	float BoxY = Data->GridSize.Y * GridLength * 0.5f - 1.f;
	OverlapBox->SetBoxExtent(FVector(BoxX, BoxY, 50.f));
	
	int GridDecalRangeMultiplier = 5;
	float GridDecalRange = GridLength * GridDecalRangeMultiplier;
	
	float DecalRangeX = BoxX + GridDecalRange;
	float DecalRangeY = BoxY + GridDecalRange;
	GridDecalComponent->DecalSize = FVector(200.f, DecalRangeX, DecalRangeY);	// 투사깊이, x, y
}

void AFactoryPlacePreview::SetPlacementValid(const bool bIsValid)
{
	if (bIsValid == bIsPlacementValid)
		return;
	
	bIsPlacementValid = bIsValid;
	
	if (PreviewDynamicMaterial)
	{
		FLinearColor TargetColor = bIsValid ? FLinearColor::Gray : FLinearColor::Red;
		PreviewDynamicMaterial->SetVectorParameterValue(TEXT("Color"), TargetColor);
	}
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
	
	// 물체 겹침 검사
	TArray<AActor*> OverlappedActors;
	OverlapBox->GetOverlappingActors(OverlappedActors);
	
	bool bIsOverlapping = false;
	for (AActor* OverlappedActor : OverlappedActors)
	{
		// TODO : 배치 객체 클래스를 만든 후 클래스 타입이나 콜리전채널로 검사
		// 프리뷰 본인이나 바닥이 아닌 객체 감지시 배치 차단
		if (OverlappedActor->ActorHasTag(TEXT("Player"))) continue;
		if (OverlappedActor && OverlappedActor != this && !OverlappedActor->ActorHasTag(TEXT("Floor")))
		{
			bIsOverlapping = true;
			break;
		}
	}
	
	SetPlacementValid(!bIsOverlapping);
}

