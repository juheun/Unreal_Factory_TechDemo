// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryNameplateWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS(Abstract)
class FACTORYTECHDEMO_API UFactoryNameplateWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Factory|UI")
	void SetFacilityName(const FText& InName);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FacilityNameText;
};
