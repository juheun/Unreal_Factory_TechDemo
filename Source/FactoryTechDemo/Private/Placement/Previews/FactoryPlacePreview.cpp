// Fill out your copyright notice in the Description page of Project Settings.


#include "Placement/Previews/FactoryPlacePreview.h"

#include "Core/FactoryDeveloperSettings.h"
#include "Placement/FactoryObjectData.h"
#include "Placement/FactoryPlaceObjectBase.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Logistics/FactoryArrowMeshComponent.h"
#include "Logistics/Ports/FactoryPortComponentBase.h"

AFactoryPlacePreview::AFactoryPlacePreview()
{
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
	
	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->SetupAttachment(RootComponent);
	OverlapBox->SetGenerateOverlapEvents(true);
	OverlapBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	
	OverlapBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	OverlapBox->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	OverlapBox->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
}

void AFactoryPlacePreview::InitPreview(const UFactoryObjectData* Data, AFactoryLogisticsObjectBase* InOriginalObject)
{
	if (!Data) return;
	ObjectData = Data;
	OriginalObject = InOriginalObject;

	ClearSpawnedArrows();

	if (Data->PlaceObjectBP)
	{
		if (AFactoryPlaceObjectBase* PlaceObjectBase = Data->PlaceObjectBP->GetDefaultObject<AFactoryPlaceObjectBase>())
		{
			MeshComponent->SetStaticMesh(PlaceObjectBase->GetStaticMesh());
			SetupPortArrows(Data, PlaceObjectBase);
		}
	}

	SetupVisualsAndCollisions(Data);
}

EOverlapValidityResult AFactoryPlacePreview::UpdateOverlapValidity()
{
	TArray<AFactoryPlaceObjectBase*> OverlappedObjects = GetOverlappingPlaceObjects();
	
	if (OverlappedObjects.Num() == 0)
	{
		CurrentValidity = EOverlapValidityResult::Valid;
		return CurrentValidity;
	}
	
	CurrentValidity = EOverlapValidityResult::Invalid;
	return CurrentValidity;
}

void AFactoryPlacePreview::SetVisualValidity(const bool bIsGlobalValid)
{
	if (CurrentValidity == EOverlapValidityResult::Skip)
	{
		SetActorHiddenInGame(true);
		return;
	}
	
	SetActorHiddenInGame(false);
	
	if (PreviewDynamicMaterial)
    {
        // 전체 경로의 유효성에 따라 색상 변경
        FLinearColor TargetColor = 
        	(bIsGlobalValid && CurrentValidity != EOverlapValidityResult::Invalid) ? FLinearColor::Green : FLinearColor::Red;
        PreviewDynamicMaterial->SetVectorParameterValue(TEXT("Color"), TargetColor);
    }
}

TArray<AFactoryPlaceObjectBase*> AFactoryPlacePreview::GetOverlappingPlaceObjects() const
{
	TArray<AActor*> OverlappedActors;
	OverlapBox->GetOverlappingActors(OverlappedActors);
    
	TArray<AFactoryPlaceObjectBase*> ResultObjects;
    
	for (AActor* OverlappedActor : OverlappedActors)
	{
		if (!OverlappedActor || OverlappedActor == this) continue;
		if (OverlappedActor->ActorHasTag(TEXT("Player")) || OverlappedActor->ActorHasTag(TEXT("Floor")) || OverlappedActor->IsA<APlayerController>()) continue;
       
		if (AFactoryPlaceObjectBase* PlaceObj = Cast<AFactoryPlaceObjectBase>(OverlappedActor))
		{
			ResultObjects.Add(PlaceObj);
		}
	}
	return ResultObjects;
}

void AFactoryPlacePreview::ClearSpawnedArrows()
{
	for (UFactoryArrowMeshComponent* Arrow : SpawnedArrows)
	{
		if (Arrow) Arrow->DestroyComponent();
	}
	SpawnedArrows.Empty();
}

void AFactoryPlacePreview::SetupPortArrows(const UFactoryObjectData* Data, const AFactoryPlaceObjectBase* CDO)
{
	TArray<UFactoryArrowMeshComponent*> BPArrows;
	CDO->GetComponents<UFactoryArrowMeshComponent>(BPArrows, true);
    
	for (UFactoryArrowMeshComponent* Arrow : BPArrows)
	{
		FTransform ParentTransform = Arrow->GetAttachParent() ? Arrow->GetAttachParent()->GetRelativeTransform() : FTransform::Identity;
		FTransform FinalTransform = Arrow->GetRelativeTransform() * ParentTransform;
		CloneAndAttachArrow(Arrow, FinalTransform);
	}
	
	if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(Data->PlaceObjectBP))
	{
		if (USimpleConstructionScript* SCS = BPClass->SimpleConstructionScript)
		{
			for (USCS_Node* Node : SCS->GetAllNodes())
			{
				if (Node->ComponentClass && Node->ComponentClass->IsChildOf(UFactoryPortComponentBase::StaticClass()))
				{
					if (UFactoryPortComponentBase* BPPortTemplate = Cast<UFactoryPortComponentBase>(Node->GetActualComponentTemplate(BPClass)))
					{
						if (UFactoryArrowMeshComponent* TargetArrow = BPPortTemplate->GetPortDirArrowComponent())
						{
							FTransform PortTransform = BPPortTemplate->GetRelativeTransform();
							FTransform ArrowLocalTransform = TargetArrow->GetRelativeTransform();
							FTransform FinalTransform = ArrowLocalTransform * PortTransform;
							CloneAndAttachArrow(TargetArrow, FinalTransform);
						}
					}
				}
			}
		}
	}
}

void AFactoryPlacePreview::SetupVisualsAndCollisions(const UFactoryObjectData* Data)
{
	const UFactoryDeveloperSettings* DeveloperSettings = GetDefault<UFactoryDeveloperSettings>();
	if (!DeveloperSettings) return;
    
	// 머티리얼 세팅
	if (UMaterialInterface* PreviewMaterial = DeveloperSettings->GetPlacePreviewMaterial())
	{
		MeshComponent->SetMaterial(0, PreviewMaterial);
		PreviewDynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0);
	}
    
	// 데칼 세팅
	if (GridDecalComponent)
	{
		if (UMaterialInterface* DecalMaterial = DeveloperSettings->GetGridDecalMaterial())
		{
			GridDecalComponent->SetDecalMaterial(DecalMaterial);
		}
	}
    
	// 사이즈 계산
	float GridLength = DeveloperSettings->GetGridLength();
	float BoxX = Data->GridSize.X * GridLength * 0.5f - 1.f;
	float BoxY = Data->GridSize.Y * GridLength * 0.5f - 1.f;
    
	if (OverlapBox)
	{
		OverlapBox->SetBoxExtent(FVector(BoxX, BoxY, 500.f));
	}
    
	int GridDecalRangeMultiplier = 5;
	float GridDecalRange = GridLength * GridDecalRangeMultiplier;
    
	if (GridDecalComponent)
	{
		GridDecalComponent->DecalSize = FVector(200.f, BoxX + GridDecalRange, BoxY + GridDecalRange);
	}
}

void AFactoryPlacePreview::CloneAndAttachArrow(UFactoryArrowMeshComponent* SourceArrow, const FTransform& RelativeTransform)
{
	if (!SourceArrow) return;

	UFactoryArrowMeshComponent* NewArrow = NewObject<UFactoryArrowMeshComponent>(this);
	NewArrow->SetArrowColor(SourceArrow->ArrowColor);
	NewArrow->ArrowSize = SourceArrow->ArrowSize;
	NewArrow->SetArrowLength(SourceArrow->ArrowLength);
    
	NewArrow->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	NewArrow->RegisterComponent();
	NewArrow->SetRelativeTransform(RelativeTransform);
    
	NewArrow->SetHiddenInGame(false);
	
	SpawnedArrows.Add(NewArrow);
}

