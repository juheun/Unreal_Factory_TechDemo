// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryInteractionWidget.generated.h"

struct FInteractionOption;
class UVerticalBox;
/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryInteractionWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetInteractionTextList(const TArray<FInteractionOption>& Options, int32 SelectedIndex);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> InteractionListPanel;
};
