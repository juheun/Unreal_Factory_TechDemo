// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Facility/FactoryFacilitySlotWidget.h"

#include "UI/Core/FactoryItemDragDropOperation.h"
#include "Logistics/FactoryLogisticsObjectBase.h"

void UFactoryFacilitySlotWidget::InitSlotIdentity(AFactoryLogisticsObjectBase* InOwnerFacility, bool bInIsInput,
	int32 InSlotIndex)
{
	OwnerFacility = InOwnerFacility;
	bIsInputSlot = bInIsInput;
	FacilitySlotIndex = InSlotIndex;
}

bool UFactoryFacilitySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                              UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	// 읽기 전용 슬롯(벨트 등)이면 드롭 무시
	if (!bIsInteractable) return false;

	if (UFactoryItemDragDropOperation* DragOp = Cast<UFactoryItemDragDropOperation>(InOperation))
	{
		OnSlotDropped.Broadcast(this, DragOp->ItemData, DragOp->DraggedAmount, DragOp->SourceSlotWidget);
		return true;
	}

	return false;
}
