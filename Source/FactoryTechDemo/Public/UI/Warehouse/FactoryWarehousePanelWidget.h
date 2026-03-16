// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryWarehousePanelWidget.generated.h"

class UFactoryWarehouseSlotWidget;
class UWrapBox;
class UFactoryItemData;
/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryWarehousePanelWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitPanel();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	UFUNCTION()
	void OnWarehouseItemChanged(const UFactoryItemData* ItemData, int32 Amount);
	
	UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
	TSubclassOf<UFactoryWarehouseSlotWidget> SlotWidgetBP;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWrapBox> WarehouseWrapBox;
	
private:
	UPROPERTY(Transient)
	TMap<const UFactoryItemData*, UFactoryWarehouseSlotWidget*> ActiveSlots;
};
