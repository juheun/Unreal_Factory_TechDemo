#include "FactoryPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FactoryCharacter.h"
#include "FactoryTopViewPawn.h"
#include "DrawDebugHelpers.h"
#include "FactoryObjectData.h"
#include "FactoryPlacePreview.h"

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
}

void AFactoryPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        // 뷰 모드 전환
        EnhancedInputComponent->BindAction(ToggleViewModeAction, ETriggerEvent::Started, this, &AFactoryPlayerController::OnToggleViewMode);
        // 배치모드 고스트 회전
        EnhancedInputComponent->BindAction(GhostRotateAction, ETriggerEvent::Started, this, &AFactoryPlayerController::RotatePlacementPreview);
    }
}

void AFactoryPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    
    //TODO : 배치모드가 실행되었을때만 돌도록 추후 수정
    FVector GhostBuildingLocation = GetPlacementObjectLocation();
    DrawDebugSphere(GetWorld(), GhostBuildingLocation, 50.f, 26, FColor::Red);
    if (CurrentPlacementPreview)
    {
        CurrentPlacementPreview->SetActorLocation(GhostBuildingLocation);
    }
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
    EViewModeType NewMode;

    // 뷰 모드 전환 준비
    if (CurrentViewMode == EViewModeType::NormalView)
    {
        if (CurrentPawn)
        {
            CachedTopViewPawn->SetActorLocation(CurrentPawn->GetActorLocation());
        }
        CachedTopViewPawn->SetActorHiddenInGame(false);
        TargetPawn = CachedTopViewPawn;
        NewMode = EViewModeType::TopView;
    }
    else
    {
        CachedTopViewPawn->SetCameraPerspective(true);
        TargetPawn = CachedNormalViewCharacter;
        NewMode = EViewModeType::NormalView;
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
        
        if (NewMode == EViewModeType::NormalView)
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
    
    if (CurrentViewMode == EViewModeType::NormalView)
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
    
    return CalculateSnappedLocation(HitResult.Location, CurrentPlacementPreview->GetObjectData()->GridSize);
}

FVector AFactoryPlayerController::CalculateSnappedLocation(FVector InRawLocation, FIntPoint GridSize) const
{
    auto SnapValue = [](float Raw, int32 Size) -> float
    {
        // TODO : 매직넘버 처리
        // 기본 100단위 그리드 시작점 (칸의 왼쪽/위쪽 선)
        float GridStart = FMath::FloorToFloat(Raw / 100.0f) * 100.0f;
        
        // 칸수가 홀수면 50유닛(중앙)만큼, 짝수면 0만큼 오프셋
        float Offset = (Size % 2 != 0) ? 50.0f : 0.0f;
        
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
    if (!CurrentPlacementPreview) return;
    
    float CurrentYaw = CurrentPlacementPreview->GetActorRotation().Yaw;
    float NextYaw = FMath::GridSnap(CurrentYaw + 90.f, 90.f);
    NextYaw = FRotator::NormalizeAxis(NextYaw);
    
    CurrentPlacementPreview->SetActorRotation(FRotator(0.f, NextYaw, 0.f));
}

