// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/FactoryDataSubsystem.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Items/FactoryRecipeData.h"

void UFactoryDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	CacheAllRecipesData();
}

void UFactoryDataSubsystem::CacheAllRecipesData()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// 검색 필터 설정
	FARFilter Filter;
	Filter.ClassPaths.Add(UFactoryRecipeData::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true; // 자식 클래스도 포함

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	// 찾은 에셋들을 로드하여 캐싱 맵에 분류
	for (const FAssetData& AssetData : AssetDataList)
	{
		// 실제 에셋 메모리에 로드
		if (UFactoryRecipeData* Recipe = Cast<UFactoryRecipeData>(AssetData.GetAsset()))
		{
			// 설비 -> 레시피 매핑
			if (Recipe->RequiredFacility)
			{
				FacilityToRecipesMap.FindOrAdd(Recipe->RequiredFacility).Recipes.Add(Recipe);
			}

			// 재료 아이템 -> 사용처(레시피) 매핑
			for (const FRecipeIngredient& Input : Recipe->Inputs)
			{
				if (Input.ItemData)
				{
					ItemUsageMap.FindOrAdd(Input.ItemData).Recipes.Add(Recipe);
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("FactoryDataSubsystem: Cached %d Recipes."), AssetDataList.Num());
}

TArray<UFactoryRecipeData*> UFactoryDataSubsystem::GetRecipeDatasForFacility(
	UFactoryFacilityItemData* FacilityType) const
{
	if (FacilityType && FacilityToRecipesMap.Contains(FacilityType))
	{
		return FacilityToRecipesMap[FacilityType].Recipes;
	}
	return TArray<UFactoryRecipeData*>();
}

TArray<UFactoryRecipeData*> UFactoryDataSubsystem::GetRecipesUsingItem(UFactoryItemData* Item) const
{
	if (Item && ItemUsageMap.Contains(Item))
	{
		return ItemUsageMap[Item].Recipes;
	}
	return TArray<UFactoryRecipeData*>();
}
