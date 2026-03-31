// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/FactoryTopViewPawn.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Cooker/CookDependency.h"
#include "Player/FactoryPlayerController.h"
#include "Player/Input/FactoryInputConfig.h"

AFactoryTopViewPawn::AFactoryTopViewPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 4000.f;
    SpringArm->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
    SpringArm->bDoCollisionTest = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->ProjectionMode = ECameraProjectionMode::Orthographic;

    SetActorEnableCollision(false);
}

void AFactoryTopViewPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        AFactoryPlayerController* PlayerController = Cast<AFactoryPlayerController>(GetController());
        if (!PlayerController || !PlayerController->GetInputConfig()) return;
        
        UFactoryInputConfig* Config = PlayerController->GetInputConfig();
        
        EnhancedInputComponent->BindAction(Config->TopViewMoveAction, ETriggerEvent::Triggered, this, &AFactoryTopViewPawn::Move);
        EnhancedInputComponent->BindAction(Config->TopViewDragMoveAction, ETriggerEvent::Triggered, this, &AFactoryTopViewPawn::DragPan);
        EnhancedInputComponent->BindAction(Config->TopViewZoomAction, ETriggerEvent::Triggered, this, &AFactoryTopViewPawn::Zoom);
        EnhancedInputComponent->BindAction(Config->TopViewRotateAction, ETriggerEvent::Started, this, &AFactoryTopViewPawn::RotateCamera);
    }
}

void AFactoryTopViewPawn::BeginPlay()
{
    Super::BeginPlay();
    
    TargetOrthoWidth = Camera->OrthoWidth;
    TargetYaw = GetActorRotation().Yaw;
}

void AFactoryTopViewPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 목표에 따라 줌 로직 수행
    if (!FMath::IsNearlyEqual(Camera->OrthoWidth, TargetOrthoWidth))
    {
        Camera->OrthoWidth = FMath::FInterpTo(Camera->OrthoWidth, TargetOrthoWidth, DeltaTime, ZoomInterpSpeed);
    }

    // 목표에 따라 회전 로직 수행
    FRotator CurrentRot = GetActorRotation();
    FRotator TargetRot = FRotator(0.0f, TargetYaw, 0.0f);
    
    if (!CurrentRot.Equals(TargetRot, 0.1f))
    {
        FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotationInterpSpeed);
        SetActorRotation(NewRot);
    }
}

void AFactoryTopViewPawn::SetCameraPerspective(bool bIsPerspective)
{
    if (bIsPerspective)
    {
        Camera->ProjectionMode = ECameraProjectionMode::Perspective;
    }
    else
    {
        Camera->ProjectionMode = ECameraProjectionMode::Orthographic;
    }
}

void AFactoryTopViewPawn::Move(const FInputActionValue& Value)
{
    FVector2D MoveVector = Value.Get<FVector2D>();
    
    FVector Offset = (GetActorForwardVector() * MoveVector.Y) + (GetActorRightVector() * MoveVector.X);
    AddActorWorldOffset(Offset * MoveSpeed * GetWorld()->GetDeltaSeconds());
}

void AFactoryTopViewPawn::DragPan(const FInputActionValue& Value)
{
    FVector2D DragDelta = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        if (AFactoryPlayerController* PC = Cast<AFactoryPlayerController>(Controller))
        {
            if (PC->GetIsStorageOpen())
            {
                return;
            }
            if (PC->GetCurrentPlacementMode() == EPlacementMode::MultipleControl)
            {
                return;
            }
        }

        // 유저가 땅을 잡고 끄는 느낌을 위해 -1을 곱함
        FVector Offset = (GetActorForwardVector() * -DragDelta.Y) + (GetActorRightVector() * -DragDelta.X);
    
        // 줌 배율에 따라 보정
        float ZoomRatio = Camera->OrthoWidth / MaxOrthoWidth;
        AddActorWorldOffset(Offset * PanSpeed * ZoomRatio);
    }
}

void AFactoryTopViewPawn::Zoom(const FInputActionValue& Value)
{
    float ZoomAxis = Value.Get<float>(); // 휠 업: 양수, 휠 다운: 음수

    // 휠 방향에 따라 목표 너비를 더하거나 뺌
    TargetOrthoWidth -= (ZoomAxis * ZoomStep);
    
    // 최소/최대 줌 제한
    TargetOrthoWidth = FMath::Clamp(TargetOrthoWidth, MinOrthoWidth, MaxOrthoWidth);
}

void AFactoryTopViewPawn::RotateCamera(const FInputActionValue& Value)
{
    // 반시계 방향으로 회전
    TargetYaw = FRotator::NormalizeAxis(TargetYaw - 90.0f);
}

