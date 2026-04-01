// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryRecipeBillboardWidget.generated.h"

class UImage;
class UTexture2D;

UCLASS(Abstract)
class FACTORYTECHDEMO_API UFactoryRecipeBillboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Factory|UI")
	void SetRecipeIcon(UTexture2D* InIcon);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RecipeIconImage;
};
