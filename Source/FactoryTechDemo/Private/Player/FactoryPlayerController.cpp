#include "Player/FactoryPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/OverlapResult.h"
#include "Interface/FactoryInteractable.h"
#include "Inventory/FactoryInventoryWidget.h"
#include "Inventory/FactoryInventoryComponent.h"
#include "Player/FactoryCharacter.h"
#include "Player/FactoryTopViewPawn.h"
#include "Settings/FactoryBuildingSettings.h"
#include "Placement/FactoryObjectData.h"
#include "Placement/FactoryPlacePreview.h"
#include "Placement/FactoryPlaceObjectBase.h"

AFactoryPlayerController::AFactoryPlayerController()
{
    InventoryComponent = CreateDefaultSubobject<UFactoryInventoryComponent>(TEXT("InventoryComponent"));
}

#pragma region 엔진 라이프 사이클

void AFactoryPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 1. 인풋 서브시스템 설정
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext) Subsystem->AddMappingContext(DefaultMappingContext, 0);
        if (MouseMappingContext) Subsystem->AddMappingContext(MouseMappingContext, 1);
        if (QuickSlotContext) Subsystem->AddMappingContext(QuickSlotContext, 1);
    }
    
    // 2. 액터 캐싱 및 생성
    CachedNormalViewCharacter = Cast<AFactoryCharacter>(GetCharacter());
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    CachedTopViewPawn = GetWorld()->SpawnActor<AFactoryTopViewPawn>(
        AFactoryTopViewPawn::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    CachedTopViewPawn->SetActorHiddenInGame(true);

    // 3. 게임 설정 로드
    if (const UFactoryBuildingSettings* Settings = GetDefault<UFactoryBuildingSettings>())
    {
        GridLength = Settings->GetGridLength();
    }
    
    // 4. UI 초기화
    if (IsLocalController() && InventoryWidgetBP)
    {
        InventoryWidget = CreateWidget<UFactoryInventoryWidget>(this, InventoryWidgetBP);
        if (InventoryWidget && InventoryComponent)
        {
            InventoryWidget->InitInventory(InventoryComponent, InventoryColumns);
        }
    }
}

void AFactoryPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    
    if (bIsPlaceMode && CurrentPlacePreview.IsValid())
    {
        FVector GhostBuildingLocation = GetPlacementObjectLocation();
        CurrentPlacePreview->SetActorLocation(GhostBuildingLocation);
    }
}

void AFactoryPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInputComponent->BindAction(ToggleViewModeAction, ETriggerEvent::Started, this, &AFactoryPlayerController::OnToggleViewMode);
        EnhancedInputComponent->BindAction(PreviewObjectRotateAction, ETriggerEvent::Started, this, &AFactoryPlayerController::RotatePlacementPreview);
        EnhancedInputComponent->BindAction(PlaceObjectAction, ETriggerEvent::Started, this, &AFactoryPlayerController::PlaceObject);
        EnhancedInputComponent->BindAction(PlaceObjectCancelAction, ETriggerEvent::Started, this, &AFactoryPlayerController::CancelPlaceObject);
        EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &AFactoryPlayerController::ToggleInventoryWidget);
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AFactoryPlayerController::OnInteract);

        for (int i = 0; i < QuickSlotActionArr.Num(); i++)
        {
            if (QuickSlotActionArr[i])
            {
                EnhancedInputComponent->BindAction(QuickSlotActionArr[i], ETriggerEvent::Started, this, &AFactoryPlayerController::ExecuteQuickSlotAction, i);
            }
        }
    }
}

#pragma endregion 

#pragma region UI 및 입력 상태 제어

void AFactoryPlayerController::ToggleInventoryWidget()
{
    if (InventoryWidget)
    {
        if (bIsInventoryOpen) InventoryWidget->RemoveFromParent();
        else InventoryWidget->AddToViewport();
    }
    bIsInventoryOpen = !bIsInventoryOpen;
    UpdateInputState();
}

void AFactoryPlayerController::UpdateInputState()
{
    if (bIsInventoryOpen)
    {
        FInputModeGameAndUI InputMode;
        if (InventoryWidget) InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
    else
    {
        if (CurrentViewMode == EFactoryViewModeType::TopView)
        {
            SetInputMode(FInputModeGameAndUI());
            bShowMouseCursor = true;
        }
        else
        {
            SetInputMode(FInputModeGameOnly());
            bShowMouseCursor = false;
        }
    }
}

#pragma endregion 

#pragma region 뷰 모드 전환

void AFactoryPlayerController::OnToggleViewMode()
{
    if (!CachedNormalViewCharacter.IsValid() || !CachedTopViewPawn.IsValid()) return;

    const float BlendTime = 0.3f;
    APawn* TargetPawn = (CurrentViewMode == EFactoryViewModeType::NormalView) ? 
        Cast<APawn>(CachedTopViewPawn.Get()) : Cast<APawn>(CachedNormalViewCharacter.Get());

    if (CurrentViewMode == EFactoryViewModeType::NormalView)
    {
        CachedTopViewPawn->SetActorLocation(GetPawn()->GetActorLocation());
        CachedTopViewPawn->SetActorHiddenInGame(false);
    }
    else
    {
        CachedTopViewPawn->SetCameraPerspective(true);
    }
    
    DisableInput(this);
    SetViewTargetWithBlend(TargetPawn, BlendTime);

    FTimerHandle ViewChangeTimerHandle;
    GetWorldTimerManager().SetTimer(ViewChangeTimerHandle, [this, TargetPawn]()
    {
        if(!IsValid(this)) return;

        Possess(TargetPawn);
        EFactoryViewModeType NewMode = (CurrentViewMode == EFactoryViewModeType::NormalView) ? 
            EFactoryViewModeType::TopView : EFactoryViewModeType::NormalView;

        if (NewMode == EFactoryViewModeType::NormalView)
        {
            CachedTopViewPawn->SetActorHiddenInGame(true);
        }
        else
        {
            CachedTopViewPawn->SetActorHiddenInGame(false);
            CachedTopViewPawn->SetCameraPerspective(false);
        }

        CurrentViewMode = NewMode; 
        UpdateInputState();
        EnableInput(this);
    }, BlendTime, false);
}

#pragma endregion 

#pragma region 배치 시스템 (Placement)

void AFactoryPlayerController::SetCurrentPlacePreview(UFactoryObjectData* Data)
{
    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    
    if (!Data)
    {
        bIsPlaceMode = false;
        if (Subsystem && PlacementContext) Subsystem->RemoveMappingContext(PlacementContext);
        if (CurrentPlacePreview.IsValid()) { CurrentPlacePreview->Destroy(); CurrentPlacePreview = nullptr; }
        CurrentPlacePreviewData = nullptr;
        return;
    }

    bIsPlaceMode = true;
    if (Subsystem && PlacementContext) Subsystem->AddMappingContext(PlacementContext, 2);
    
    CurrentPlacePreviewData = Data;
    CurrentPlacePreview = GetWorld()->SpawnActor<AFactoryPlacePreview>();
    if (CurrentPlacePreview.IsValid()) CurrentPlacePreview->InitPreview(Data);
}

FVector AFactoryPlayerController::GetPlacementObjectLocation() const
{
    FHitResult HitResult;
    bool bHit = false;
    
    if (CurrentViewMode == EFactoryViewModeType::NormalView)
    {
        FVector CameraLocation; FRotator CameraRotation;
        GetPlayerViewPoint(CameraLocation, CameraRotation);
        FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * MaxBuildTraceDistance);
        
        FCollisionQueryParams Params; Params.AddIgnoredActor(GetPawn());
        bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_GameTraceChannel1, Params);
        
        if (!bHit)
        {
            bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceEnd, TraceEnd + (FVector::DownVector * MaxBuildTraceDistance), ECC_GameTraceChannel1, Params);
        }
    }
    else
    {
        bHit = GetHitResultUnderCursor(ECC_GameTraceChannel1, false, HitResult);
    }
    
    if (!bHit) return FVector::ZeroVector;
    return CalculateSnappedLocation(HitResult.Location, CurrentPlacePreviewData->GridSize);
}

FVector AFactoryPlayerController::CalculateSnappedLocation(FVector InRawLocation, FIntPoint GridSize) const
{
    auto SnapValue = [this](float Raw, int32 Size) -> float
    {
        float GridStart = FMath::FloorToFloat(Raw / GridLength) * GridLength;
        float Offset = (Size % 2 != 0) ? GridLength * 0.5f : 0.0f;
        return GridStart + Offset;
    };

    return FVector(SnapValue(InRawLocation.X, GridSize.X), SnapValue(InRawLocation.Y, GridSize.Y), InRawLocation.Z);
}

void AFactoryPlayerController::RotatePlacementPreview()
{
    if (!CurrentPlacePreview.IsValid()) return;
    float NextYaw = FMath::GridSnap(CurrentPlacePreview->GetActorRotation().Yaw + 90.f, 90.f);
    CurrentPlacePreview->SetActorRotation(FRotator(0.f, FRotator::NormalizeAxis(NextYaw), 0.f));
}

void AFactoryPlayerController::PlaceObject()
{
    if (CurrentPlacePreview.IsValid() && CurrentPlacePreview->GetPlacementValid() && CurrentPlacePreviewData)
    {
        FActorSpawnParameters Params; Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        GetWorld()->SpawnActor<AFactoryPlaceObjectBase>(CurrentPlacePreviewData->PlaceObjectBP, CurrentPlacePreview->GetActorLocation(), CurrentPlacePreview->GetActorRotation(), Params);
        SetCurrentPlacePreview(nullptr);
    }
}

void AFactoryPlayerController::CancelPlaceObject()
{
    SetCurrentPlacePreview(nullptr);
}

#pragma endregion 

#pragma region 퀵슬롯 시스템

void AFactoryPlayerController::ExecuteQuickSlotAction(int32 SlotIndex)
{
    if (bIsPlaceMode) return;
    if (QuickSlotActionArr.IsValidIndex(SlotIndex) && QuickSlotObjectDataArr.IsValidIndex(SlotIndex) && QuickSlotObjectDataArr[SlotIndex])
    {
        SetCurrentPlacePreview(QuickSlotObjectDataArr[SlotIndex]);
    }
}

#pragma endregion 

#pragma region 상호작용 시스템

void AFactoryPlayerController::OnInteract()
{
    if (bIsPlaceMode || bIsInventoryOpen) return;
    
    TScriptInterface<IFactoryInteractable> Target = FindBestInteractable();
    if (Target)
    {
        Target->Interact(GetPawn());
    }
}

TScriptInterface<IFactoryInteractable> AFactoryPlayerController::FindBestInteractable()
{
    if (CurrentViewMode == EFactoryViewModeType::NormalView)
    {
        APawn* ControlledPawn = GetPawn();
        if (!ControlledPawn) return nullptr;

        FVector CharacterLoc = ControlledPawn->GetActorLocation();
    
        TArray<FOverlapResult> Overlaps;
        FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionRange);
    
        // 범위 내 모든 액터 탐색
        GetWorld()->OverlapMultiByChannel(Overlaps, CharacterLoc, FQuat::Identity, ECC_Visibility, Sphere);

        TScriptInterface<IFactoryInteractable> BestTarget = nullptr;
        float ClosestDistanceSqr = TNumericLimits<float>::Max(); // 가장 작은 거리를 찾기 위해 최댓값으로 초기화

        for (auto& Result : Overlaps)
        {
            AActor* OverlapActor = Result.GetActor();
            if (OverlapActor && OverlapActor->Implements<UFactoryInteractable>())
            {
                // 캐릭터와 타겟 사이의 거리 계산 (비교용이므로 루트 연산이 없는 Squared 버전 사용)
                float CurrentDistSqr = FVector::DistSquared(CharacterLoc, OverlapActor->GetActorLocation());

                // 현재까지 찾은 대상 중 가장 가까운지 확인
                if (CurrentDistSqr < ClosestDistanceSqr)
                {
                    ClosestDistanceSqr = CurrentDistSqr;
                    BestTarget = OverlapActor;
                }
            }
        }
        return BestTarget;
    }
    else
    {
        FHitResult HitResult;
        if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
        {
            if (AActor* HitActor = HitResult.GetActor())
            {
                if (HitActor->Implements<UFactoryInteractable>())
                {
                    return HitActor;
                }
            }
        }
    }

    return nullptr;
}

#pragma endregion
