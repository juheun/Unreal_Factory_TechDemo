// Fill out your copyright notice in the Description page of Project Settings.


#include "Placement/FactoryPlacePreview.h"

#include "Settings/FactoryDeveloperSettings.h"
#include "Placement/FactoryObjectData.h"
#include "Placement/FactoryPlaceObjectBase.h"
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
	MeshComponent->SetReceivesDecals(false);
	MeshComponent->SetForceDisableNanite(true);
	
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
	OverlapBox->SetGenerateOverlapEvents(true);
	OverlapBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
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
	
	const UFactoryDeveloperSettings* DeveloperSettings = GetDefault<UFactoryDeveloperSettings>();
	if (!DeveloperSettings) return;
	
	UMaterialInterface* PreviewMaterial = DeveloperSettings->GetPlacePreviewMaterial();
	
	MeshComponent->SetMaterial(0, PreviewMaterial);
	PreviewDynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0);
	
	float GridLength = DeveloperSettings->GetGridLength();
	
	float BoxX = Data->GridSize.X * GridLength * 0.5f - 1.f;	// 바로 옆 그리드와 붙는것 방지하기 위해 1 빼줌
	float BoxY = Data->GridSize.Y * GridLength * 0.5f - 1.f;
	OverlapBox->SetBoxExtent(FVector(BoxX, BoxY, 500.f));
	
	int GridDecalRangeMultiplier = 5;
	float GridDecalRange = GridLength * GridDecalRangeMultiplier;
	
	float DecalRangeX = BoxX + GridDecalRange;
	float DecalRangeY = BoxY + GridDecalRange;
	GridDecalComponent->DecalSize = FVector(200.f, DecalRangeX, DecalRangeY);	// 투사깊이, x, y
}

bool AFactoryPlacePreview::UpdateOverlapValidity()
{
	TArray<AActor*> OverlappedActors;
	OverlapBox->GetOverlappingActors(OverlappedActors);
    
	bool bIsOverlapping = false;
	for (AActor* OverlappedActor : OverlappedActors)
	{
		if (OverlappedActor->ActorHasTag(TEXT("Player"))) continue;
		if (OverlappedActor && OverlappedActor != this && !OverlappedActor->ActorHasTag(TEXT("Floor")))
		{
			bIsOverlapping = true;
			break;
		}
	}

	bIsPlacementValid = !bIsOverlapping;
	return bIsPlacementValid;
}

void AFactoryPlacePreview::SetVisualValidity(const bool bIsGlobalValid)
{
	if (PreviewDynamicMaterial)
    {
        // 전체 경로의 유효성에 따라 색상 변경
        FLinearColor TargetColor = bIsGlobalValid ? FLinearColor::Gray : FLinearColor::Red;
        PreviewDynamicMaterial->SetVectorParameterValue(TEXT("Color"), TargetColor);
    }
}
