// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FactoryObjectData.generated.h"

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
	
	//TODO: 추후 AActor가 아닌 실제 고스트, 배치오브젝트 클래스로 변경할것 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classes")
	TSubclassOf<class AFactoryPlaceObjectBase> PlaceObjectBP;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
	FIntPoint GridSize = FIntPoint(1, 1);
};
