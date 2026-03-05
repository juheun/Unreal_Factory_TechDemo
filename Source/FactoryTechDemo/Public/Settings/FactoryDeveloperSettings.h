// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FactoryDeveloperSettings.generated.h"

class AFactoryItemVisual;

UENUM(BlueprintType)
enum class EFactoryPoolType : uint8
{
	ItemVisual,
	BeltPreview,
};

UCLASS(Config = Game, DefaultConfig, meta=(DisplayName = "Factory Developer Settings"))
class FACTORYTECHDEMO_API UFactoryDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	static const UFactoryDeveloperSettings* Get() {return GetDefault<UFactoryDeveloperSettings>();}
	
	UMaterialInterface* GetPlacePreviewMaterial() const {return PreviewObjectMaterial.LoadSynchronous();};
	
	TSubclassOf<AActor> GetPoolClass(EFactoryPoolType PoolType) const
	{
		return PoolClassMap.Contains(PoolType) ? PoolClassMap[PoolType] : nullptr;
	}
	
	float GetGridLength() const {return GridLength;}

private:
	UPROPERTY(Config, EditAnywhere, Category="Visuals")
	TSoftObjectPtr<UMaterialInterface> PreviewObjectMaterial;
	
	UPROPERTY(Config, EditAnywhere, Category="Pooling")
	TMap<EFactoryPoolType, TSubclassOf<AActor>> PoolClassMap;
	
	UPROPERTY(Config, EditAnywhere, Category="Grid")
	float GridLength = 100.f;
};
