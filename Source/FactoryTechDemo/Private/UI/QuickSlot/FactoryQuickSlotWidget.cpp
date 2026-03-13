// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/QuickSlot/FactoryQuickSlotWidget.h"

#include "Components/Image.h"
#include "Player/Component/FactoryQuickSlotComponent.h"
#include "UI/Core/FactoryItemDragDropOperation.h"
#include "Components/TextBlock.h"
#include "Items/FactoryFacilityItemData.h"
#include "Placement/FactoryObjectData.h"

void UFactoryQuickSlotWidget::InitQuickSlot(UFactoryQuickSlotComponent* QuickSlotComp, int32 InHotkeyIndex)
{
	LinkedQuickSlotComp = QuickSlotComp;
	HotkeyIndex = InHotkeyIndex;

	if (HotkeyText)
	{
		// 인덱스 0번 = 단축키 1번
		HotkeyText->SetText(FText::AsNumber((HotkeyIndex + 1) % 10));
	}
	
	if (UFactoryQuickSlotComponent* Comp = LinkedQuickSlotComp.Get())
	{
		Comp->OnQuickSlotDataChanged.AddDynamic(this, &UFactoryQuickSlotWidget::OnDataChanged);

		// 생성 직후 현재 컴포넌트가 가진 데이터로 초기 렌더링
		Comp->BroadcastQuickSlotChange(HotkeyIndex);
	}
}

void UFactoryQuickSlotWidget::UpdateSlotVisual(const UFactoryItemData* ItemData, int32 Amount)
{
	CurrentItemData = ItemData;
	CurrentAmount = Amount;
	
	if (CurrentItemData == nullptr)
	{
		if (ItemIcon) ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		if (AmountText) AmountText->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		if (ItemIcon)
		{
			if (CurrentItemData->ItemIcon)
			{
				ItemIcon->SetBrushFromTexture(CurrentItemData->ItemIcon);
			}
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
			
			if (CurrentAmount < 0)
			{
				ItemIcon->SetRenderOpacity(1.0f);
				if (AmountText) AmountText->SetVisibility(ESlateVisibility::Hidden);
			}
			else if (CurrentAmount == 0)
			{
				ItemIcon->SetRenderOpacity(0.5f);
				if (AmountText)
				{
					AmountText->SetText(FText::AsNumber(CurrentAmount));
					AmountText->SetVisibility(ESlateVisibility::Visible);
				}
			}
			else
			{
				ItemIcon->SetRenderOpacity(1.0f);
				if (AmountText)
				{
					// 수량이 0이어도 0이라고 표시해줌
					AmountText->SetText(FText::AsNumber(CurrentAmount));
					AmountText->SetVisibility(ESlateVisibility::Visible);
				}
			}
		}
	}
}

void UFactoryQuickSlotWidget::OnDataChanged(int32 Index, UFactoryObjectData* Data, int32 Amount)
{
	if (Index == HotkeyIndex)
	{
		const UFactoryItemData* DisplayItem = Data ? Data->RepresentingItemData : nullptr;
        
		int32 DisplayAmount = Amount;
		if (Data && !Data->bRefundItemOnDestroy)
		{
			DisplayAmount = -1;
		}
		UpdateSlotVisual(DisplayItem, DisplayAmount);
	}
}

bool UFactoryQuickSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	UFactoryItemDragDropOperation* DragOperation = Cast<UFactoryItemDragDropOperation>(InOperation);
	if (!DragOperation) return false;
	
	// 다른 퀵슬롯에서 온 경우 스왑
	if (UFactoryQuickSlotWidget* SourceQuickSlot = Cast<UFactoryQuickSlotWidget>(DragOperation->SourceSlotWidget))
	{
		if (SourceQuickSlot != this && LinkedQuickSlotComp.IsValid())
		{
			LinkedQuickSlotComp->SwapQuickSlotData(SourceQuickSlot->HotkeyIndex, this->HotkeyIndex);
		}
		return true;
	}
	
	const UFactoryFacilityItemData* FacilityData = Cast<UFactoryFacilityItemData>(DragOperation->ItemData);
	if (FacilityData && FacilityData->PlacementData)
	{
		// 설비가 맞다면, 퀵슬롯 컴포넌트에게 등록을 요청
		if (LinkedQuickSlotComp.IsValid())
		{
			LinkedQuickSlotComp->SetQuickSlotData(HotkeyIndex, FacilityData->PlacementData);
		}
		return true;
	}
	return false;	// 설비가 아닌경우
}
