// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/QuickSlot/FactoryQuickSlotWidget.h"
#include "Player/Component/FactoryQuickSlotComponent.h"
#include "UI/Core/FactoryItemDragDropOperation.h"
#include "Components/TextBlock.h"

void UFactoryQuickSlotWidget::InitQuickSlot(UFactoryQuickSlotComponent* QuickSlotComp, int32 InHotkeyIndex)
{
	LinkedQuickSlotComp = QuickSlotComp;
	HotkeyIndex = InHotkeyIndex;

	if (HotkeyText)
	{
		// 인덱스 0번 = 단축키 1번
		HotkeyText->SetText(FText::AsNumber(HotkeyIndex + 1));
	}
}

bool UFactoryQuickSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	UFactoryItemDragDropOperation* DragOperation = Cast<UFactoryItemDragDropOperation>(InOperation);
	if (!DragOperation) return false;

	// TODO : 추후 FactoryQuickSlotComponent에 SetQuickSlotData 함수를 만들어서 연동
	
	/* if (LinkedQuickSlotComp.IsValid())
	{
		LinkedQuickSlotComp->SetQuickSlotData(HotkeyIndex, DragOperation->ItemData);
	}
	*/
	
	return true;
}