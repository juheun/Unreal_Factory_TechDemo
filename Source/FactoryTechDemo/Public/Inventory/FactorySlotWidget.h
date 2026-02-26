// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FFactorySlot.h"
#include "Blueprint/UserWidget.h"
#include "FactorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UFactoryInventoryComponent;
class UFactoryItemData;

UENUM(BlueprintType)
enum class EFactorySlotType : uint8
{
	Inventory UMETA(DisplayName = "Inventory"),
	Warehouse UMETA(DisplayName = "Warehouse"),
	QuickSlot UMETA(DisplayName = "QuickSlot"),
};

UCLASS()
class FACTORYTECHDEMO_API UFactorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "FactorySlot")
	void InitSlot(UFactoryInventoryComponent* InventoryComponent, EFactorySlotType Type, int32 Index);
	
protected:
	UFUNCTION(BlueprintCallable, Category = "FactorySlot")
	void OnSlotDataChanged(int32 UpdatedSlotIndex, FFactorySlot UpdatedSlotData);
	
	UFUNCTION(BlueprintCallable, Category = "FactorySlot")
	void UpdateSlotInfo(const UFactoryItemData* ItemData, int32 Amount);
	
	virtual FReply NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmountText;
	
	UPROPERTY()
	TWeakObjectPtr<UFactoryInventoryComponent> LinkedInventory;	 // 이 슬롯을 들고 있는 인벤토리 컴포넌트
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FactorySlot")
	EFactorySlotType SlotType;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FactorySlot")
	int32 SlotIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FactorySlot")
	TObjectPtr<const UFactoryItemData> CurrentItemData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FactorySlot")
	int32 CurrentAmount;
};
