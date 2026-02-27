// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryInteractionWidget.generated.h"

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryInteractionWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetInteractionText(const FText& Text);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> InteractionTextBlock;
};
