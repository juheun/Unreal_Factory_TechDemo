// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryFacilityWorldUIComponent.h"
#include "UFactoryFacilityBlockWarningComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryFacilityBlockWarningComponent : public UFactoryFacilityWorldUIComponent
{
	GENERATED_BODY()

public:
	UFactoryFacilityBlockWarningComponent();
	
	UFUNCTION()
	void OnFacilityBlockCallback(bool bIsBlocked);
	
protected:
	virtual void UpdateUIPlacement(float DeltaTime, const FVector& CameraLoc, const FVector& CameraForward, const FVector& OwnerLoc) override;
	
	UPROPERTY(EditDefaultsOnly, Category="Factory|Billboard")
	float DefaultBillboardZHeight = 400.f;
	UPROPERTY(EditDefaultsOnly, Category="Factory|Billboard")
	float BillboardZHeight = 300.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Factory|Billboard")
	float TopViewScreenOffset = 150.f;
};
