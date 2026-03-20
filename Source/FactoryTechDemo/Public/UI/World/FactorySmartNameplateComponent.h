// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryFacilityWorldUIComponent.h"
#include "Player/FactoryPlayerController.h"
#include "FactorySmartNameplateComponent.generated.h"


enum class EFactoryViewModeType : uint8;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactorySmartNameplateComponent : public UFactoryFacilityWorldUIComponent
{
	GENERATED_BODY()

public:
	UFactorySmartNameplateComponent();

	virtual void BeginPlay() override;
	
protected:
	virtual void OnViewModeChanged(EFactoryViewModeType NewViewMode) override;
	virtual void UpdateUIPlacement(float DeltaTime, const FVector& CameraLoc, const FVector& CameraForward, const FVector& OwnerLoc) override;
	
	UPROPERTY(EditDefaultsOnly, Category="Factory|Nameplate")
	float FixedZHeight = 150.f;
	
private:
	float ExtentX = 50.f;
	float ExtentY = 50.f;
};
