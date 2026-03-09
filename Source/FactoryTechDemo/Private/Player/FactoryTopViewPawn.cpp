// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/FactoryTopViewPawn.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Player/FactoryPlayerController.h"
#include "Player/Input/FactoryInputConfig.h"

// Sets default values
AFactoryTopViewPawn::AFactoryTopViewPawn()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
    Camera->OrthoWidth = CamWidth;

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
        
        // TODO : 구현
        // EnhancedInputComponent->BindAction(InputConfig->TopViewDragMoveAction, ETriggerEvent::Triggered, this, &AFactoryTopViewPawn::);
        // EnhancedInputComponent->BindAction(InputConfig->TopViewRotateAction, ETriggerEvent::Triggered, this, &AFactoryTopViewPawn::);
        // EnhancedInputComponent->BindAction(InputConfig->TopViewZoomAction, ETriggerEvent::Triggered, this, &AFactoryTopViewPawn::);
    }
}

void AFactoryTopViewPawn::BeginPlay()
{
    Super::BeginPlay();
}

void AFactoryTopViewPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
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
