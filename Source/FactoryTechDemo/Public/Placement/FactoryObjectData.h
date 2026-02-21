// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FactoryObjectData.generated.h"

class AFactoryPlaceObjectBase;

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryObjectData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText ObjectName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classes")
	TSubclassOf<AFactoryPlaceObjectBase> PlaceObjectBP;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
	FIntPoint GridSize = FIntPoint(1, 1);
};
