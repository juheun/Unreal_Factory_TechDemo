// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Core/FactoryBaseSlotWidget.h"
#include "FactoryFacilitySlotWidget.generated.h"

class UFactoryFacilitySlotWidget;
class UFactoryItemData;
class AFactoryLogisticsObjectBase;

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnFacilitySlotDropped, 
	UFactoryFacilitySlotWidget*, SlotWidget, const UFactoryItemData*, ItemData, int32, Amount, UFactoryBaseSlotWidget*, SourceWidget);
UCLASS()
class FACTORYTECHDEMO_API UFactoryFacilitySlotWidget : public UFactoryBaseSlotWidget
{
	GENERATED_BODY()
	
public:
	void InitSlotIdentity(AFactoryLogisticsObjectBase* InOwnerFacility, bool bInIsInput, int32 InSlotIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FactorySlot|Identity")
	TWeakObjectPtr<AFactoryLogisticsObjectBase> OwnerFacility;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FactorySlot|Identity")
	bool bIsInputSlot = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FactorySlot|Identity")
	int32 FacilitySlotIndex = 0;
	
	UPROPERTY(BlueprintAssignable, Category = "FactorySlot|Event")
	FOnFacilitySlotDropped OnSlotDropped;
	
protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
