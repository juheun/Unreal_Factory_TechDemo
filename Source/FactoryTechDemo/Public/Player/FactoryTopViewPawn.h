// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FactoryTopViewPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class FACTORYTECHDEMO_API AFactoryTopViewPawn : public APawn
{
    GENERATED_BODY()

public:
    AFactoryTopViewPawn();

protected:
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    void SetCameraPerspective(bool bIsPerspective);
    
private:
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    TObjectPtr<USpringArmComponent> SpringArm;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    TObjectPtr<UCameraComponent> Camera;
    float CamWidth = 3000.f;
};
