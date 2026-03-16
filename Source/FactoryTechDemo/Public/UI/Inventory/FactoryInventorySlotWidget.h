// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Core/FactoryBaseSlotWidget.h"
#include "UI/Core/FFactorySlot.h"
#include "FactoryInventorySlotWidget.generated.h"

class UFactoryInventoryComponent;

UCLASS()
class FACTORYTECHDEMO_API UFactoryInventorySlotWidget : public UFactoryBaseSlotWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "FactorySlot")
	void InitInventorySlot(UFactoryInventoryComponent* InventoryComponent, int32 Index);
	
	UFactoryInventoryComponent* GetInventoryComponent() const { return LinkedInventory.Get(); }
	int32 GetSlotIndex() const { return SlotIndex; }
	
protected:
	UFUNCTION(BlueprintCallable, Category = "FactorySlot")
	void OnSlotDataChanged(int32 UpdatedSlotIndex, FFactorySlot UpdatedSlotData);

	// 진짜 아이템 이동 처리
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FactorySlot")
	int32 SlotIndex;
	
private:
	UPROPERTY()
	TWeakObjectPtr<UFactoryInventoryComponent> LinkedInventory;
};
