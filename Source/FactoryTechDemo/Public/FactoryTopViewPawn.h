// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FactoryTopViewPawn.generated.h"

UCLASS()
class FACTORYTECHDEMO_API AFactoryTopViewPawn : public APawn
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    AFactoryTopViewPawn();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    void SetCameraPerspective(bool bIsPerspective);

private:
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    TObjectPtr<class USpringArmComponent> SpringArm;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    TObjectPtr<class UCameraComponent> Camera;
    float CamWidth = 3000.f;
};
