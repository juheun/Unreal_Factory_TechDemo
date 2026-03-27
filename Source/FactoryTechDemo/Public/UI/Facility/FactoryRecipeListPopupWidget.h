// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryRecipeListPopupWidget.generated.h"

class UFactoryRecipeData;
class UFactoryMachineRecipeWidget;
class UButton;
class UScrollBox;

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryRecipeListPopupWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitPopup(const TArray<UFactoryRecipeData*>& Recipes);
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseRecipeListPanelBtn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> RecipeListScrollBox;
	
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UFactoryMachineRecipeWidget> RecipeWidgetClass;
	
private:
	UFUNCTION()
	void OnCloseRecipeListClicked();
};
