// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Core/FactoryBaseSlotWidget.h"
#include "FactoryQuickSlotWidget.generated.h"

class UFactoryQuickSlotComponent;
class UFactoryObjectData;

UCLASS()
class FACTORYTECHDEMO_API UFactoryQuickSlotWidget : public UFactoryBaseSlotWidget
{
	GENERATED_BODY()

public:
	void InitQuickSlot(UFactoryQuickSlotComponent* QuickSlotComp, int32 InHotkeyIndex);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HotkeyText;
	
	virtual void UpdateSlotVisual(const UFactoryItemData* ItemData, int32 Amount) override;
	
	UFUNCTION()
	void OnDataChanged(int32 Index, UFactoryObjectData* Data, int32 Amount);

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	UPROPERTY()
	TWeakObjectPtr<UFactoryQuickSlotComponent> LinkedQuickSlotComp;
	
	int32 HotkeyIndex;
};