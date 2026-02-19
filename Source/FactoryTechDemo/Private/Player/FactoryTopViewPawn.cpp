// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/FactoryTopViewPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

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

    SetActorTickEnabled(false);
    SetActorEnableCollision(false);
}

// Called when the game starts or when spawned
void AFactoryTopViewPawn::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AFactoryTopViewPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AFactoryTopViewPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
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
