// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Player/Input/FactoryPlayerController.h"
#include "FactoryFacilityWorldUIComponent.generated.h"


enum class EFactoryViewModeType : uint8;

UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryFacilityWorldUIComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UFactoryFacilityWorldUIComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, Category="Factory|WorldUI")
	virtual void WakeUp();
	
	UFUNCTION(BlueprintCallable, Category="Factory|WorldUI")
	virtual void GoToSleep();
	
protected:
	UFUNCTION()
	virtual void OnViewModeChanged(EFactoryViewModeType NewViewMode);
	
	// 자식 클래스에서 이동/회전 로직 구현
	virtual void UpdateUIPlacement(float DeltaTime, const FVector& CameraLoc, const FVector& CameraForward, const FVector& OwnerLoc) {};
	
	UPROPERTY(EditDefaultsOnly, Category="Factory|WorldUI")
	float InterpolationSpeed = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Factory|WorldUI")
	float CullingDotThreshold = 0.4f;
	
	UPROPERTY(EditDefaultsOnly, Category="Factory|WorldUI")
	float MaxWidgetScale = 1.2f;
	
	UPROPERTY(EditDefaultsOnly, Category="Factory|WorldUI")
	float MinWidgetScale = 0.5f;
	
	EFactoryViewModeType CachedCurrentViewMode = EFactoryViewModeType::NormalView;
	bool bIsAwake = false;
	
	UPROPERTY()
	TWeakObjectPtr<AFactoryTopViewPawn> CachedTopViewPawn;
};
