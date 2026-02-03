// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ViewModeType.h"
#include "FactoryPlayerController.generated.h"

/**
 *
 */
UCLASS()
class FACTORYTECHDEMO_API AFactoryPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AFactoryPlayerController();
    
    virtual void PlayerTick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    EViewModeType CurrentViewMode = EViewModeType::NormalView;

private:
    UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UInputMappingContext> MouseMappingContext;

    UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UInputAction> ToggleViewModeAction;
    UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UInputAction> GhostRotateAction;

    UPROPERTY()
    TObjectPtr<class AFactoryCharacter> CachedNormalViewCharacter; // 3인칭 뷰 캐릭터
    UPROPERTY()
    TObjectPtr<class AFactoryTopViewPawn> CachedTopViewPawn;    // 위에서 내려다보는 폰
    
    UPROPERTY(EditAnywhere, Category = Input)
    TObjectPtr<AActor> CachedGhostBuilding;    //TODO : 설치 전 배치모드의 액터. 추후에 클래스 변경 및 동적으로 받을 수 있게 변경

    const float MaxBuildTraceDistance = 1000.f;
    
    void OnToggleViewMode();
    FVector GetBuildingPlacementLocation();
    void RotationGhostBuilding();
    
    //bool IsBuildingMode() const;
    
    virtual void SetupInputComponent() override;
};
