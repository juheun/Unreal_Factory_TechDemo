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
    GridDecalComponent->SetRelativeLocation(FVector(0.f, 0.f, 1.f));   // 데칼 적용을 위해 살짝 들어올림
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
    // --- 1. CDO 기반 네이티브 화살표 추출 ---
    TArray<UFactoryArrowMeshComponent*> BPArrows;
    CDO->GetComponents<UFactoryArrowMeshComponent>(BPArrows, true);
    
    for (UFactoryArrowMeshComponent* Arrow : BPArrows)
    {
        FTransform AccumTransform = Arrow->GetRelativeTransform();
        USceneComponent* CurrentParent = Arrow->GetAttachParent();
        
        while (CurrentParent && CurrentParent != CDO->GetRootComponent())
        {
            AccumTransform = AccumTransform * CurrentParent->GetRelativeTransform();
            CurrentParent = CurrentParent->GetAttachParent();
        }
        AttachArrowWithSpecs(AccumTransform, Arrow->ArrowColor, Arrow->ArrowSize, Arrow->ArrowLength);
    }
    
    // --- 2. 블루프린트 SCS 기반 노드 탐색 ---
    UClass* CurrentClass = Data->PlaceObjectBP;

    while (CurrentClass && CurrentClass->IsChildOf(AActor::StaticClass()))
    {
        if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(CurrentClass))
        {
            if (USimpleConstructionScript* SCS = BPClass->SimpleConstructionScript)
            {
                for (USCS_Node* Node : SCS->GetAllNodes())
                {
                    if (!Node || !Node->ComponentClass) continue;
                    
                    // SCS 내 부모 트랜스폼 누적을 위한 람다 함수
                    auto GetAccumulatedTransform = [&](const FTransform& BaseTransform) -> FTransform
                    {
                        FTransform Accum = BaseTransform;
                        USCS_Node* ParentNode = SCS->FindParentNode(Node);
                        while (ParentNode)
                        {
                            if (USceneComponent* ParentComp = Cast<USceneComponent>(ParentNode->GetActualComponentTemplate(BPClass)))
                            {
                                Accum = Accum * ParentComp->GetRelativeTransform();
                            }
                            ParentNode = SCS->FindParentNode(ParentNode);
                        }
                        return Accum;
                    };

                    // 케이스 A: 수동 추가 화살표
                    if (Node->ComponentClass->IsChildOf(UFactoryArrowMeshComponent::StaticClass()))
                    {
                        if (UFactoryArrowMeshComponent* BPArrowTemplate = Cast<UFactoryArrowMeshComponent>(Node->GetActualComponentTemplate(BPClass)))
                        {
                            FTransform FinalTransform = GetAccumulatedTransform(BPArrowTemplate->GetRelativeTransform());
                            AttachArrowWithSpecs(FinalTransform, BPArrowTemplate->ArrowColor, BPArrowTemplate->ArrowSize, BPArrowTemplate->ArrowLength);
                        }
                    }
                    // 케이스 B: 포트 컴포넌트 (CDO를 활용해 스펙 안전 추출)
                    else if (Node->ComponentClass->IsChildOf(UFactoryPortComponentBase::StaticClass()))
                    {
                        if (UFactoryPortComponentBase* BPPortTemplate = Cast<UFactoryPortComponentBase>(Node->GetActualComponentTemplate(BPClass)))
                        {
                            UFactoryPortComponentBase* PortCDO = Cast<UFactoryPortComponentBase>(Node->ComponentClass->GetDefaultObject());
                            if (PortCDO && PortCDO->GetPortDirArrowComponent())
                            {
                                UFactoryArrowMeshComponent* DefaultArrow = PortCDO->GetPortDirArrowComponent();
                                
                                FTransform PortTransform = GetAccumulatedTransform(BPPortTemplate->GetRelativeTransform());
                                FTransform FinalTransform = DefaultArrow->GetRelativeTransform() * PortTransform;

                                AttachArrowWithSpecs(FinalTransform, DefaultArrow->ArrowColor, DefaultArrow->ArrowSize, DefaultArrow->ArrowLength);
                            }
                        }
                    }
                }
            }
        }
        CurrentClass = CurrentClass->GetSuperClass();
    }
}

void AFactoryPlacePreview::SetupVisualsAndCollisions(const UFactoryObjectData* Data)
{
    const UFactoryDeveloperSettings* DeveloperSettings = GetDefault<UFactoryDeveloperSettings>();
    if (!DeveloperSettings) return;
    
    if (UMaterialInterface* PreviewMaterial = DeveloperSettings->GetPlacePreviewMaterial())
    {
        MeshComponent->SetMaterial(0, PreviewMaterial);
        PreviewDynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0);
    }
    
    if (GridDecalComponent)
    {
        if (UMaterialInterface* DecalMaterial = DeveloperSettings->GetGridDecalMaterial())
        {
            GridDecalComponent->SetDecalMaterial(DecalMaterial);
        }
    }
    
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

void AFactoryPlacePreview::AttachArrowWithSpecs(const FTransform& RelativeTransform, FColor Color, float Size, float Length)
{   
    UFactoryArrowMeshComponent* NewArrow = NewObject<UFactoryArrowMeshComponent>(this);
    
    NewArrow->ArrowSize = Size;
    NewArrow->ArrowLength = Length;
    NewArrow->ArrowColor = Color;
    
    NewArrow->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    NewArrow->RegisterComponent();
    
    // 위치 세팅
    NewArrow->SetRelativeTransform(RelativeTransform);
    NewArrow->SetArrowColor(Color);
    NewArrow->SetHiddenInGame(false);
    
    SpawnedArrows.Add(NewArrow);
}