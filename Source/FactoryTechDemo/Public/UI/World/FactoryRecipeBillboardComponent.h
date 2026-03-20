// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryFacilityWorldUIComponent.h"
#include "FactoryRecipeBillboardComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryRecipeBillboardComponent : public UFactoryFacilityWorldUIComponent
{
	GENERATED_BODY()
	
public:
	UFactoryRecipeBillboardComponent();

protected:
	virtual void UpdateUIPlacement(float DeltaTime, const FVector& CameraLoc, const FVector& CameraForward, const FVector& OwnerLoc) override;

	UPROPERTY(EditDefaultsOnly, Category="Factory|Billboard")
	float BillboardZHeight = 250.f; // 아이콘은 2.5m 높이에 위치
};