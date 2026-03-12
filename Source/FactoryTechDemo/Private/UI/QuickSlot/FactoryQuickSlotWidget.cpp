// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/QuickSlot/FactoryQuickSlotWidget.h"
#include "Player/Component/FactoryQuickSlotComponent.h"
#include "UI/Core/FactoryItemDragDropOperation.h"
#include "Components/TextBlock.h"
#include "Items/FactoryFacilityItemData.h"
#include "Placement/FactoryObjectData.h"
#include "Player/Component/FactoryInventoryComponent.h"

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
		const TArray<TObjectPtr<UFactoryObjectData>>& DataArr = Comp->GetQuickSlotDataArray();
		if (DataArr.IsValidIndex(HotkeyIndex))
		{
			OnDataChanged(HotkeyIndex, DataArr[HotkeyIndex]);
		}
	}
}

void UFactoryQuickSlotWidget::OnDataChanged(int32 Index, UFactoryObjectData* Data)
{
	if (Index == HotkeyIndex)
	{
		// ObjectData에서 시각화용 ItemData 추출
		const UFactoryItemData* DisplayItem = Data ? Data->RepresentingItemData : nullptr;
        
		int32 TotalAmount = 0;
        
		// 인벤토리를 찾아서 총합 개수를 가져옵니다.
		if (DisplayItem)
		{
			if (APlayerController* PC = GetOwningPlayer())
			{
				if (UFactoryInventoryComponent* InvComp = PC->FindComponentByClass<UFactoryInventoryComponent>())
				{
					TotalAmount = InvComp->GetTotalItemAmount(DisplayItem);
				}
			}
		}
        
		// TODO : 추후 갯수가 0개여도 아이콘 사라지지 않게 수정
		UpdateSlotVisual(DisplayItem, DisplayItem ? FMath::Max(1, TotalAmount) : 0);
	}
}

bool UFactoryQuickSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	
	UFactoryItemDragDropOperation* DragOperation = Cast<UFactoryItemDragDropOperation>(InOperation);
	if (!DragOperation) return false;

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
