#include "FactoryPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FactoryCharacter.h"
#include "FactoryTopViewPawn.h"
#include "FactoryBuildingSettings.h"
#include "FactoryObjectData.h"
#include "FactoryPlacePreview.h"
#include "FactoryPlaceObjectBase.h"

AFactoryPlayerController::AFactoryPlayerController()
{
    // bShowMouseCursor = true;
}

void AFactoryPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
        if (MouseMappingContext)
        {
            Subsystem->AddMappingContext(MouseMappingContext, 1);
        }
    }
    
    // 3인칭 뷰 캐릭터 캐싱
    CachedNormalViewCharacter = Cast<AFactoryCharacter>(GetCharacter());
    
    // 탑뷰 액터 생성 및 캐싱
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    CachedTopViewPawn = GetWorld()->SpawnActor<AFactoryTopViewPawn>(
        AFactoryTopViewPawn::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams);
    CachedTopViewPawn->SetActorHiddenInGame(true);

    if (const UFactoryBuildingSettings* Settings = GetDefault<UFactoryBuildingSettings>())
    {
        GridLength = Settings->GetGridLength();
    }

}

void AFactoryPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        // 뷰 모드 전환
        EnhancedInputComponent->BindAction(
            ToggleViewModeAction, ETriggerEvent::Started, this, &AFactoryPlayerController::OnToggleViewMode);
        // 배치모드 고스트 회전
        EnhancedInputComponent->BindAction(
            PreviewObjectRotateAction, ETriggerEvent::Started, this, &AFactoryPlayerController::RotatePlacementPreview);
        // 오브젝트 배치
        EnhancedInputComponent->BindAction(
            PlaceObjectAction, ETriggerEvent::Started, this, &AFactoryPlayerController::PlaceObject);
        // 오브젝트 배치모드 시작 // TODO: 제거
        EnhancedInputComponent->BindAction(
            TemporaryStartPlaceModeAction, ETriggerEvent::Started, this, &AFactoryPlayerController::TemporaryStartPlaceMode);
    }
}

void AFactoryPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    
    if (!bIsPlaceMode || !CurrentPlacePreview) return;
    
    FVector GhostBuildingLocation = GetPlacementObjectLocation();
    CurrentPlacePreview->SetActorLocation(GhostBuildingLocation);
}

/**
 * 새로 배치할 객체를 정함
 * @param Data 새로 배치할 객체, nullptr 이면 배치모드 종료 
 */
void AFactoryPlayerController::SetCurrentPlacePreview(class UFactoryObjectData* Data)
{
    if (!Data)
    {
        bIsPlaceMode = false;
        if (CurrentPlacePreview)
        {
            CurrentPlacePreview->Destroy();
            CurrentPlacePreview = nullptr;
            CurrentPlacePreviewData = nullptr;
        }
        return;
    }
    bIsPlaceMode = true;
    CurrentPlacePreviewData = Data;
    CurrentPlacePreview = GetWorld()->SpawnActor<AFactoryPlacePreview>();
    CurrentPlacePreview->InitPreview(Data);
}

/** 
 * 3인칭과 탑뷰 모드 전환
 */
void AFactoryPlayerController::OnToggleViewMode()
{
    if (!CachedNormalViewCharacter || !CachedTopViewPawn)
    {
        return;
    }

    const float BlendTime = 0.3f;
    APawn* CurrentPawn = GetPawn();
    APawn* TargetPawn = nullptr;
    EFactoryViewModeType NewMode;

    // 뷰 모드 전환 준비
    if (CurrentViewMode == EFactoryViewModeType::NormalView)
    {
        if (CurrentPawn)
        {
            CachedTopViewPawn->SetActorLocation(CurrentPawn->GetActorLocation());
        }
        CachedTopViewPawn->SetActorHiddenInGame(false);
        TargetPawn = CachedTopViewPawn;
        NewMode = EFactoryViewModeType::TopView;
    }
    else
    {
        CachedTopViewPawn->SetCameraPerspective(true);
        TargetPawn = CachedNormalViewCharacter;
        NewMode = EFactoryViewModeType::NormalView;
    }

    // 뷰 모드 전환 시작
    DisableInput(this);
    SetViewTargetWithBlend(TargetPawn, BlendTime);

    FTimerHandle ViewChangeTimerHandle;
    GetWorldTimerManager().SetTimer(ViewChangeTimerHandle, [this, TargetPawn, NewMode]()
    {
        if(!IsValid(this))
        {
            return;
        }

        Possess(TargetPawn);
        
        if (NewMode == EFactoryViewModeType::NormalView)
        {
            bShowMouseCursor = false;
            CachedTopViewPawn->SetActorHiddenInGame(true);
            SetInputMode(FInputModeGameOnly());
        }
        else
        {
            CachedTopViewPawn->SetActorHiddenInGame(false);
            CachedTopViewPawn->SetCameraPerspective(false);
            bShowMouseCursor = true;
            SetInputMode(FInputModeGameAndUI());
        }
        EnableInput(this);
        CurrentViewMode = NewMode; 
    }, BlendTime, false);
}

/**
 * 배치모드에서 그리드에 맞게 배치될 고스트의 위치 산출
 * @return 고스트의 위치
 */
FVector AFactoryPlayerController::GetPlacementObjectLocation() const
{
    FHitResult HitResult;
    bool bHit;
    
    if (CurrentViewMode == EFactoryViewModeType::NormalView)
    {
        FVector CameraLocation;
        FRotator CameraRotation;
        GetPlayerViewPoint(CameraLocation, CameraRotation);
        FVector TraceStart = CameraLocation;
        FVector TraceEnd = TraceStart + (CameraRotation.Vector() * MaxBuildTraceDistance);
        
        FCollisionQueryParams CollisionQueryParams;
        CollisionQueryParams.AddIgnoredActor(GetPawn());
        
        bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult, TraceStart, TraceEnd, ECC_GameTraceChannel1, CollisionQueryParams);
        
        // 만약 일정 거리 감지못하면 아래쪽 검사
        if (!bHit)
        {
            FVector DownTraceStart = TraceEnd;
            FVector DownTraceEnd = DownTraceStart + (FVector::DownVector * MaxBuildTraceDistance);
            bHit = GetWorld()->LineTraceSingleByChannel(
                HitResult, DownTraceStart, DownTraceEnd, ECC_GameTraceChannel1, CollisionQueryParams);
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
        // 그리드 시작점 (칸의 왼쪽/위쪽 선)
        float GridStart = FMath::FloorToFloat(Raw / GridLength) * GridLength;
        
        // 칸수가 홀수면 중앙만큼, 짝수면 0만큼 오프셋
        float Offset = (Size % 2 != 0) ? GridLength * 0.5f : 0.0f;
        
        return GridStart + Offset;
    };

    FVector Snapped;
    Snapped.X = SnapValue(InRawLocation.X, GridSize.X);
    Snapped.Y = SnapValue(InRawLocation.Y, GridSize.Y);
    Snapped.Z = InRawLocation.Z; // 높이는 바닥에 고정

    return Snapped;
}

/**
 * 배치모드에서 고스트를 회전시킴
 */
void AFactoryPlayerController::RotatePlacementPreview()
{
    if (!CurrentPlacePreview) return;
    
    float CurrentYaw = CurrentPlacePreview->GetActorRotation().Yaw;
    float NextYaw = FMath::GridSnap(CurrentYaw + 90.f, 90.f);
    NextYaw = FRotator::NormalizeAxis(NextYaw);
    
    CurrentPlacePreview->SetActorRotation(FRotator(0.f, NextYaw, 0.f));
}

void AFactoryPlayerController::PlaceObject()
{
    // 프리뷰가 있고, 현재 위치가 배치 가능한 상태일 때만 소환
    if (CurrentPlacePreview && CurrentPlacePreview->GetPlacementValid() && CurrentPlacePreviewData)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        FVector Location = CurrentPlacePreview->GetActorLocation();
        FRotator Rotation = CurrentPlacePreview->GetActorRotation();

        // 데이터 에셋에 지정된 실제 BP 클래스 소환
        AFactoryPlaceObjectBase* PlaceObject = GetWorld()->SpawnActor<AFactoryPlaceObjectBase>(
            CurrentPlacePreviewData->PlaceObjectBP, 
            Location, 
            Rotation, 
            SpawnParams
        );
        if (PlaceObject)
        {
            PlaceObject->InitObject(CurrentPlacePreviewData);
        }

        SetCurrentPlacePreview(nullptr);
    }
}

void AFactoryPlayerController::TemporaryStartPlaceMode()
{
    if (bIsPlaceMode) return;
    if (TempObjectData)
    {
        SetCurrentPlacePreview(TempObjectData);
    }
}

