// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Core/FactoryBaseSlotWidget.h"
#include "FactoryWarehouseSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryWarehouseSlotWidget : public UFactoryBaseSlotWidget
{
	GENERATED_BODY()
	
public:
	void InitWarehouseSlot(const UFactoryItemData* ItemData, int32 Amount);
	
	virtual void UpdateSlotVisual(const UFactoryItemData* ItemData, int32 Amount) override;
	
	const UFactoryItemData* GetItemData() const { return SlotItemData; }
	
protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
