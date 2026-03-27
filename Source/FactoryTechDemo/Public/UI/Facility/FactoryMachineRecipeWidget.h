// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryMachineRecipeWidget.generated.h"

class UFactoryBaseSlotWidget;
class UHorizontalBox;
class UImage;
class UTextBlock;
class UFactoryRecipeData;
/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryMachineRecipeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Init(const UFactoryRecipeData* RecipeData);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> InputSlotsContainer;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UFactoryBaseSlotWidget> OutputSlot;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ProcessingTimeText;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UFactoryBaseSlotWidget> SlotWidgetClass;
};
