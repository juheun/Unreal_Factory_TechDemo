// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FFactoryInventorySlot.h"
#include "Blueprint/UserWidget.h"
#include "FactorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UFactoryInventoryComponent;

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
	void OnSlotDataChanged(int32 UpdatedSlotIndex, FFactoryInventorySlot UpdatedSlotData);
	
	UFUNCTION(BlueprintCallable, Category = "FactorySlot")
	void UpdateSlotInfo(FName ItemID, int32 Amount);
	
	virtual FReply NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmountText;
	
	TWeakObjectPtr<UFactoryInventoryComponent> LinkedInventory;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FactorySlot")
	EFactorySlotType SlotType;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FactorySlot")
	int32 SlotIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FactorySlot")
	FName CurrentItemID;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FactorySlot")
	int32 CurrentAmount;
};
