// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Warehouse/FactoryWarehousePanelWidget.h"

#include "Components/WrapBox.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"
#include "UI/Warehouse/FactoryWarehouseSlotWidget.h"

void UFactoryWarehousePanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (UFactoryWarehouseSubsystem* Warehouse = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>())
	{
		Warehouse->OnWarehouseItemChanged.AddDynamic(this, &UFactoryWarehousePanelWidget::OnWarehouseItemChanged);
	}
}

void UFactoryWarehousePanelWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (UFactoryWarehouseSubsystem* Warehouse = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>())
	{
		Warehouse->OnWarehouseItemChanged.RemoveDynamic(this, &UFactoryWarehousePanelWidget::OnWarehouseItemChanged);
	}
}

void UFactoryWarehousePanelWidget::InitPanel()
{
	if (!WarehouseWrapBox || !SlotWidgetBP) return;

	UFactoryWarehouseSubsystem* Warehouse = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
	if (!Warehouse) return;

	const auto& InventoryData = Warehouse->GetWarehouseInventory();

	// 기존에 떠있는 슬롯들 갱신 및 0개짜리 청소
	TArray<const UFactoryItemData*> ItemsToRemove;
	for (auto& Pair : ActiveSlots)
	{
		const UFactoryItemData* ItemData = Pair.Key;
		UFactoryWarehouseSlotWidget* SlotWidget = Pair.Value;

		int32 CurrentAmount = Warehouse->GetItemAmount(ItemData);
		
		if (CurrentAmount <= 0)
		{
			// 0개가 된 슬롯 청소
			SlotWidget->RemoveFromParent(); 
			ItemsToRemove.Add(ItemData);
		}
		else
		{
			// 갱신
			SlotWidget->UpdateSlotVisual(ItemData, CurrentAmount); 
		}
	}

	for (const UFactoryItemData* ItemToRemove : ItemsToRemove)
	{
		ActiveSlots.Remove(ItemToRemove);
	}

	// 새로운 아이템만 위젯 생성
	for (const auto& Pair : InventoryData)
	{
		const UFactoryItemData* ItemData = Pair.Key;
		int32 Amount = Pair.Value;

		if (Amount > 0 && !ActiveSlots.Contains(ItemData))
		{
			if (UFactoryWarehouseSlotWidget* NewSlot = CreateWidget<UFactoryWarehouseSlotWidget>(this, SlotWidgetBP))
			{
				NewSlot->InitWarehouseSlot(ItemData, Amount);
				WarehouseWrapBox->AddChildToWrapBox(NewSlot);
				ActiveSlots.Add(ItemData, NewSlot);
			}
		}
	}
}

void UFactoryWarehousePanelWidget::OnWarehouseItemChanged(const UFactoryItemData* ItemData, int32 Amount)
{
	if (UFactoryWarehouseSlotWidget** FoundSlot = ActiveSlots.Find(ItemData))
	{
		(*FoundSlot)->UpdateSlotVisual(ItemData, Amount);
	}
	else if (Amount > 0 && SlotWidgetBP)
	{
		if (UFactoryWarehouseSlotWidget* NewSlot = CreateWidget<UFactoryWarehouseSlotWidget>(this, SlotWidgetBP))
		{
			NewSlot->InitWarehouseSlot(ItemData, Amount);
			WarehouseWrapBox->AddChildToWrapBox(NewSlot);
			ActiveSlots.Add(ItemData, NewSlot);
		}
	}
}
