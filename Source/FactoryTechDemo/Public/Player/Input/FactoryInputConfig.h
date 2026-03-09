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
	// 글로벌 액션
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Global")
	TObjectPtr<UInputAction> ToggleViewModeAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Global")
	TObjectPtr<UInputAction> ToggleInventoryAction;
	
	// NormalView 이동 제어
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NormalViewMove")
	TObjectPtr<UInputAction> NormalViewMoveAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NormalViewMove")
	TObjectPtr<UInputAction> NormalViewLookAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NormalViewMove")
	TObjectPtr<UInputAction> NormalViewJumpAction;
	
	// TopView 이동 제어
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TopViewMove")
	TObjectPtr<UInputAction>  TopViewDragMoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TopViewMove")
	TObjectPtr<UInputAction>  TopViewRotateAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TopViewMove")
	TObjectPtr<UInputAction>  TopViewZoomAction;
	
	// 건설 및 물류망 제어
	UPROPERTY(EditAnywhere, Category = "Placement") 
	TObjectPtr<UInputAction> PlaceObjectAction;
	UPROPERTY(EditAnywhere, Category = "Placement") 
	TObjectPtr<UInputAction> PlaceObjectCancelAction;
	UPROPERTY(EditAnywhere, Category = "Placement") 
	TObjectPtr<UInputAction> PlaceObjectRotateAction;
	UPROPERTY(EditAnywhere, Category = "Placement") 
	TObjectPtr<UInputAction> ToggleBeltPlaceModeAction;
	UPROPERTY(EditAnywhere, Category = "Placement") 
	TObjectPtr<UInputAction> ToggleRetrieveModeAction;
	UPROPERTY(EditAnywhere, Category = "Placement") 
	TObjectPtr<UInputAction> EnterMoveModeAction;
	
	// 상호작용 제어
	UPROPERTY(EditAnywhere, Category = "interaction") 
	TObjectPtr<UInputAction> InteractAction;
	
	// 퀵슬롯
	UPROPERTY(EditAnywhere, Category = "QuickSlot")
	TArray<TObjectPtr<UInputAction>> QuickSlotActionArr;
};
