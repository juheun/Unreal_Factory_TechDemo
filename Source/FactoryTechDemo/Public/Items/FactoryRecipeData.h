// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FactoryRecipeData.generated.h"

class UFactoryItemData;
class UFactoryFacilityItemData;

USTRUCT(BlueprintType)
struct FRecipeIngredient
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TObjectPtr<UFactoryItemData> ItemData;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	int32 Amount;
};

/**
 * 공장 설비에서 아이템을 가공할 때 사용하는 레시피 데이터
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryRecipeData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TObjectPtr<UFactoryFacilityItemData> RequiredFacility;	//요구 설비
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TArray<FRecipeIngredient> Inputs;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FRecipeIngredient Output;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	int ProcessingTime = 2;	//생산에 걸리는 시간 (초)
};
