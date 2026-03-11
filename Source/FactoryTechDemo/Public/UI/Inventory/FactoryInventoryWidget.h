// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryInventoryWidget.generated.h"

class UFactoryInventoryComponent;
class UFactoryInventorySlotWidget;
class UUniformGridPanel;

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitInventory(UFactoryInventoryComponent* InventoryComponent, int32 Columns = 5);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UFactoryInventorySlotWidget> SlotWidgetBP;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> InventoryGridPanel;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UFactoryInventorySlotWidget>> SlotWidgets;
};
