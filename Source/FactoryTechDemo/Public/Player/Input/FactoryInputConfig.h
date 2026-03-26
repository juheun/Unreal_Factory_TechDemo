// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FactoryInputConfig.generated.h"

class UInputAction;

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// Global Idle (기본 대기 상태)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Global Idle")
	TObjectPtr<UInputAction> ToggleViewModeAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Global Idle")
	TObjectPtr<UInputAction> ToggleInventoryAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Idle") 
	TObjectPtr<UInputAction> ToggleBeltPlaceModeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Global Idle") 
	TArray<TObjectPtr<UInputAction>> QuickSlotActionArr;
	
	// BaseView - NormalView 이동 제어
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseView|Normal")
	TObjectPtr<UInputAction> NormalViewMoveAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseView|Normal")
	TObjectPtr<UInputAction> NormalViewLookAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseView|Normal")
	TObjectPtr<UInputAction> NormalViewJumpAction;
	// BaseView - TopView 이동 제어
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseView|Top")
	TObjectPtr<UInputAction>  TopViewMoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseView|Top")
	TObjectPtr<UInputAction>  TopViewDragMoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseView|Top")
	TObjectPtr<UInputAction>  TopViewRotateAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BaseView|Top")
	TObjectPtr<UInputAction>  TopViewZoomAction;
	
	// Specific - Normal
	UPROPERTY(EditAnywhere, Category = "Specific|Normal") 
	TObjectPtr<UInputAction> InteractAction;
	UPROPERTY(EditAnywhere, Category = "Specific|Normal")
	TObjectPtr<UInputAction> InteractScrollAction;
	UPROPERTY(EditAnywhere, Category = "Specific|Normal") 
	TObjectPtr<UInputAction> ToggleRetrieveModeAction;
	// Specific - Top
	UPROPERTY(EditAnywhere, Category = "Specific|Top") 
	TObjectPtr<UInputAction> EnterMoveModeAction;	// 설비 롱탭
	
	// 배치모드 공통 액션
	UPROPERTY(EditAnywhere, Category = "Placement") 
	TObjectPtr<UInputAction> PlaceObjectAction;
	UPROPERTY(EditAnywhere, Category = "Placement") 
	TObjectPtr<UInputAction> PlaceObjectCancelAction;
	UPROPERTY(EditAnywhere, Category = "Placement") 
	TObjectPtr<UInputAction> PlaceObjectRotateAction;
};
