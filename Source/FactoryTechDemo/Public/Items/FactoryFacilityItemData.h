// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryItemData.h"
#include "FactoryFacilityItemData.generated.h"

class UFactoryObjectData;

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryFacilityItemData : public UFactoryItemData
{
	GENERATED_BODY()
	
public:
	UFactoryFacilityItemData()
	{
		ItemCategory = EFactoryItemCategory::Facility;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
	TObjectPtr<UFactoryObjectData> PlacementData;
};
