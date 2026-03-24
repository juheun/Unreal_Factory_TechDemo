// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryPlacePreview.h"
#include "Logistics/FactoryLogisticsTypes.h"
#include "FactoryBeltPreview.generated.h"

class USplineComponent;

UCLASS()
class FACTORYTECHDEMO_API AFactoryBeltPreview : public AFactoryPlacePreview
{
	GENERATED_BODY()

public:
	AFactoryBeltPreview();

	virtual bool UpdateOverlapValidity() override;
	
	FVector GetBeltExitDirection() const;
	void SetBeltType(const EBeltType Type);
	EBeltType GetBeltType() const { return BeltType; }
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Factory|Visual")
	TMap<EBeltType, TObjectPtr<UStaticMesh>> BeltMeshMap;
	
	UPROPERTY()
	EBeltType BeltType;
};
