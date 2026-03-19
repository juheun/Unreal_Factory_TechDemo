// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Items/FactoryItemData.h"
#include "FactoryDataSubsystem.generated.h"

class UFactoryItemData;
class UFactoryFacilityItemData;
class UFactoryRecipeData;

USTRUCT()
struct FItemDataArray
{
	GENERATED_BODY()
    
	UPROPERTY()
	TArray<UFactoryItemData*> Items;
};

USTRUCT()
struct FRecipeDataArray
{
	GENERATED_BODY()
    
	UPROPERTY()
	TArray<UFactoryRecipeData*> Recipes;
};

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UFUNCTION(BlueprintCallable, Category = "Factory|Data")
	TArray<UFactoryItemData*> GetItemDatasByCategory(EFactoryItemCategory Category) const;
	
	UFUNCTION(BlueprintCallable, Category = "Factory|Data")
	TArray<UFactoryRecipeData*> GetRecipeDatasForFacility(UFactoryFacilityItemData* FacilityType) const;
	
	UFUNCTION(BlueprintCallable, Category = "Factory|Data")
	TArray<UFactoryRecipeData*> GetRecipesUsingItem(UFactoryItemData* Item) const;

private:
	UPROPERTY()
	TMap<EFactoryItemCategory, FItemDataArray> CategorizedItemsMap;
	
	UPROPERTY()
	TMap<UFactoryFacilityItemData*, FRecipeDataArray> FacilityToRecipesMap;

	UPROPERTY()
	TMap<UFactoryItemData*, FRecipeDataArray> ItemUsageMap;
	
	void CacheAllItemData();
	void CacheAllRecipesData();
};
