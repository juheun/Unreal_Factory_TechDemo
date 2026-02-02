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

    UPROPERTY()
    TObjectPtr<class AFactoryCharacter> CachedNormalViewCharacter;
    UPROPERTY()
    TObjectPtr<class AFactoryTopViewPawn> CachedTopViewPawn;

    void OnToggleViewMode();
    virtual void SetupInputComponent() override;
};
