// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FactoryBuildingSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta=(DisplayName = "Factory Build Settings"))
class FACTORYTECHDEMO_API UFactoryBuildingSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	static const UFactoryBuildingSettings* Get() {return GetDefault<UFactoryBuildingSettings>();}
	
	UMaterialInterface* GetPlacePreviewMaterial() const {return PreviewObjectMaterial.LoadSynchronous();};
	
	float GetGridLength() const {return GridLength;}

private:
	UPROPERTY(Config, EditAnywhere, Category="Visuals")
	TSoftObjectPtr<UMaterialInterface> PreviewObjectMaterial;
	
	UPROPERTY(Config, EditAnywhere, Category="Grid")
	float GridLength = 100.f;
};
