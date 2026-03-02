// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FactoryDeveloperSettings.generated.h"

class AFactoryItemVisual;
/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta=(DisplayName = "Factory Developer Settings"))
class FACTORYTECHDEMO_API UFactoryDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	static const UFactoryDeveloperSettings* Get() {return GetDefault<UFactoryDeveloperSettings>();}
	
	UMaterialInterface* GetPlacePreviewMaterial() const {return PreviewObjectMaterial.LoadSynchronous();};
	
	TSubclassOf<AFactoryItemVisual> GetItemVisualBP() const {return ItemVisualBP; }
	
	float GetGridLength() const {return GridLength;}

private:
	UPROPERTY(Config, EditAnywhere, Category="Visuals")
	TSoftObjectPtr<UMaterialInterface> PreviewObjectMaterial;
	
	UPROPERTY(Config, EditAnywhere, Category="Pooling")
	TSubclassOf<AFactoryItemVisual> ItemVisualBP;
	
	UPROPERTY(Config, EditAnywhere, Category="Grid")
	float GridLength = 100.f;
};
