// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryFacilityPanelBase.h"
#include "FactoryMachineBasePanel.generated.h"

class UFactoryRecipeListPopupWidget;
class UFactoryMachineRecipeWidget;
class UVerticalBox;
class UFactoryFacilitySlotWidget;
class AFactoryMachineBase;
struct FFactorySlot;
class UProgressBar;
class UFactoryRecipeData;
class UButton;
class UScrollBox;

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryMachineBasePanel : public UFactoryFacilityPanelBase
{
	GENERATED_BODY()
	
public:
	virtual void InitPanel(AFactoryPlaceObjectBase* PlaceObject) override;
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UFUNCTION()
	virtual void OnInputBufferUpdated(const int32 SlotIndex, const FFactorySlot SlotData);
	UFUNCTION()
	virtual void OnOutputBufferUpdated(const FFactorySlot SlotData);
	UFUNCTION()
	virtual void OnRecipeUpdated(const UFactoryRecipeData* RecipeData);
	
	UFUNCTION()
	void HandleSlotDrop(UFactoryFacilitySlotWidget* TargetSlot, const class UFactoryItemData* ItemData, int32 Amount, class UFactoryBaseSlotWidget* SourceWidget);
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> CraftingProgressBar;
	
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UFactoryFacilitySlotWidget> FacilitySlotBP;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> InputSlotContainer;	// 동적생성된 InputBufferSlot들 컨테이너
	
	UPROPERTY()
	TArray<UFactoryFacilitySlotWidget*> InputSlots;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UFactoryFacilitySlotWidget> OutputSlot;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OpenRecipeListPanelBtn;
	
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UFactoryRecipeListPopupWidget> RecipeListPopupWidgetBP;
	
	UFUNCTION()
	void OnOpenRecipeListClicked();
	
private:
	UPROPERTY()
	TObjectPtr<const UFactoryRecipeData> CachedRecipe;
	
	UPROPERTY()
	TWeakObjectPtr<AFactoryMachineBase> CachedMachine;
};
