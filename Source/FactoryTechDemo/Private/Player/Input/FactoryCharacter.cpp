// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/Input/FactoryCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Player/Input/FactoryPlayerController.h"
#include "Player/Input/FactoryInputConfig.h"

AFactoryCharacter::AFactoryCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AFactoryCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void AFactoryCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AFactoryCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        AFactoryPlayerController* PlayerController = Cast<AFactoryPlayerController>(GetController());
        if (!PlayerController || !PlayerController->GetInputConfig()) return;
        
        UFactoryInputConfig* Config = PlayerController->GetInputConfig();
        
        EnhancedInputComponent->BindAction(Config->NormalViewMoveAction, ETriggerEvent::Triggered, this, &AFactoryCharacter::Move);
        EnhancedInputComponent->BindAction(Config->NormalViewLookAction, ETriggerEvent::Triggered, this, &AFactoryCharacter::Look);
        EnhancedInputComponent->BindAction(Config->NormalViewJumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(Config->NormalViewJumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
    }
}

void AFactoryCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AFactoryCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        if (AFactoryPlayerController* PC = Cast<AFactoryPlayerController>(Controller))
        {
            if (PC->GetIsStorageOpen())
            {
                return;
            }
        }

        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}
