// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
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
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    void SetCameraPerspective(bool bIsPerspective);
    
protected:
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    
    UPROPERTY(VisibleAnywhere, Category = "Factory|Camera")
    TObjectPtr<USpringArmComponent> SpringArm;

    UPROPERTY(VisibleAnywhere, Category = "Factory|Camera")
    TObjectPtr<UCameraComponent> Camera;
    
    // 입력 콜백 함수
    void Move(const FInputActionValue& Value);          // WASD 이동
    void DragPan(const FInputActionValue& Value);       // 마우스 드래그 이동
    void Zoom(const FInputActionValue& Value);          // 휠 줌
    void RotateCamera(const FInputActionValue& Value);  // Ctrl+R 90도 회전
    
    // 조작감 관련 변수 ---
    UPROPERTY(EditAnywhere, Category = "Factory|Camera")
    float PanSpeed = 100.f; // 드래그 이동 속도
    
    UPROPERTY(EditAnywhere, Category = "Factory|Camera")
    float MoveSpeed = 2000.f; // WASD 이동 속도
    
    //줌 관련 설정
    UPROPERTY(EditAnywhere, Category = "Factory|Camera")
    float MinOrthoWidth = 1000.f;
    UPROPERTY(EditAnywhere, Category = "Factory|Camera")
    float MaxOrthoWidth = 10000.f;
    UPROPERTY(EditAnywhere, Category = "Factory|Camera")
    float ZoomStep = 500.0f;
    UPROPERTY(EditAnywhere, Category = "Factory|Camera")
    float ZoomInterpSpeed = 10.f;
    
    UPROPERTY(EditAnywhere, Category = "Factory|Camera")
    float TargetOrthoWidth;     // 줌 보간 목표값
    
    // 회전 설정
    UPROPERTY(EditAnywhere, Category = "Factory|Camera")
    float RotationInterpSpeed = 10.f;
    
    UPROPERTY(EditAnywhere, Category = "Factory|Camera")
    float TargetYaw; // 보간용 목표 회전 값
};
