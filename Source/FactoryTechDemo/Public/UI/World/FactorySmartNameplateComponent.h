// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Player/FactoryPlayerController.h"
#include "FactorySmartNameplateComponent.generated.h"


enum class EFactoryViewModeType : uint8;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactorySmartNameplateComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UFactorySmartNameplateComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, Category="Factory|Nameplate")
	void WakeUp();
	
	UFUNCTION(BlueprintCallable, Category="Factory|Nameplate")
	void GoToSleep();
	
protected:
	UFUNCTION()
	void OnViewModeChanged(EFactoryViewModeType NewViewMode);
	
	UPROPERTY(EditDefaultsOnly, Category="Factory|Nameplate")
	float FixedZHeight = 150.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Factory|Nameplate")
	float InterpolationSpeed = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Factory|Nameplate")
	float CullingDotThreshold = 10.f;
	
private:
	EFactoryViewModeType CachedCurrentViewMode = EFactoryViewModeType::NormalView;
	
	bool bIsAwake = false;
	
	// 설비 그리드 계산 후 캐싱해놓을 오프셋 거리
	float ExtentX = 50.f;
	float ExtentY = 50.f;
};
