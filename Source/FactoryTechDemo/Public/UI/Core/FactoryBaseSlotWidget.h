// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryBaseSlotWidget.generated.h"

class UTextBlock;
class UImage;
class UFactoryItemData;

/**
 * 
 */
UCLASS(Abstract)
class FACTORYTECHDEMO_API UFactoryBaseSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UFUNCTION(BlueprintCallable, Category = "FactorySlot")
	virtual void UpdateSlotVisual(const UFactoryItemData* ItemData, int32 Amount);
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmountText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FactorySlot")
	TObjectPtr<const UFactoryItemData> CurrentItemData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FactorySlot")
	int32 CurrentAmount;
};
