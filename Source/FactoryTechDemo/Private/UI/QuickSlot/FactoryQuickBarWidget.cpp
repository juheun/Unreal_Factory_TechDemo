// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/QuickSlot/FactoryQuickBarWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "UI/QuickSlot/FactoryQuickSlotWidget.h"

void UFactoryQuickBarWidget::InitQuickBar(UFactoryQuickSlotComponent* QuickSlotComponent)
{
	if (!QuickSlotComponent || !QuickSlotWidgetBP || !QuickSlotsBox) return;
	
	QuickSlotsBox->ClearChildren();
	QuickSlotWidgets.Empty();
	
	for (int i = 0; i < MaxSlots; i++)
	{
		UFactoryQuickSlotWidget* NewSlot = CreateWidget<UFactoryQuickSlotWidget>(this, QuickSlotWidgetBP);
		if (NewSlot)
		{
			NewSlot->InitQuickSlot(QuickSlotComponent, i);
			
			UHorizontalBoxSlot* BoxSlot = QuickSlotsBox->AddChildToHorizontalBox(NewSlot);
			if (BoxSlot)
			{
				BoxSlot->SetPadding(FMargin(5.f, 0.f, 5.f, 0.f));
				
				FSlateChildSize SizeRule;
				SizeRule.SizeRule = ESlateSizeRule::Fill; // Auto 대신 Fill 사용
				SizeRule.Value = 1.0f; // 모든 슬롯이 1.0의 동일한 가중치(비율)를 가짐
				BoxSlot->SetSize(SizeRule);
			}
			QuickSlotWidgets.Add(NewSlot);
		}
	}
}
