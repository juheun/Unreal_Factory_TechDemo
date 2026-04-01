// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Facility/FactoryMachineBasePanel.h"

#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Items/FactoryRecipeData.h"
#include "Logistics/Machines/FactoryMachineBase.h"
#include "Player/Component/FactoryInventoryComponent.h"
#include "Subsystems/FactoryCycleSubsystem.h"
#include "UI/Facility/FactoryFacilitySlotWidget.h"
#include "UI/Facility/FactoryMachineRecipeWidget.h"
#include "UI/Facility/FactoryRecipeListPopupWidget.h"
#include "UI/Inventory/FactoryInventorySlotWidget.h"

void UFactoryMachineBasePanel::InitPanel(AFactoryPlaceObjectBase* PlaceObject)
{
	Super::InitPanel(PlaceObject);
	
	if (AFactoryMachineBase* OldMachine = CachedMachine.Get())
	{
		// 중복방지 델리게이트 구독 해제
		OldMachine->OnInputBufferChanged.RemoveDynamic(this, &UFactoryMachineBasePanel::OnInputBufferUpdated);
		OldMachine->OnOutputBufferChanged.RemoveDynamic(this, &UFactoryMachineBasePanel::OnOutputBufferUpdated);
		OldMachine->OnCurrentRecipeChanged.RemoveDynamic(this, &UFactoryMachineBasePanel::OnRecipeUpdated);
	}
	
	CachedMachine = Cast<AFactoryMachineBase>(PlaceObject);
	
	if (AFactoryMachineBase* Machine = CachedMachine.Get())
	{
		// 델리게이트 구독
		Machine->OnInputBufferChanged.AddDynamic(this, &UFactoryMachineBasePanel::OnInputBufferUpdated);
		Machine->OnOutputBufferChanged.AddDynamic(this, &UFactoryMachineBasePanel::OnOutputBufferUpdated);
		Machine->OnCurrentRecipeChanged.AddDynamic(this, &UFactoryMachineBasePanel::OnRecipeUpdated);
		
		if (InputSlotContainer && FacilitySlotBP)
		{
			InputSlotContainer->ClearChildren();
			InputSlots.Empty();
			
			int32 InputCount = Machine->GetInputBufferSlots().Num();
			for (int32 i = 0; i < InputCount; i++)
			{
				if (UFactoryFacilitySlotWidget* NewSlot = CreateWidget<UFactoryFacilitySlotWidget>(this, FacilitySlotBP))
				{
					// Vertical Box에 자식으로 추가
					InputSlotContainer->AddChildToVerticalBox(NewSlot);
					InputSlots.Add(NewSlot);
					
					NewSlot->InitSlotIdentity(Machine, true, i);
					
					NewSlot->OnSlotDropped.AddUniqueDynamic(this, &UFactoryMachineBasePanel::HandleSlotDrop);

					// 초기 아이템 비주얼 갱신
					NewSlot->UpdateSlotVisual(Machine->GetInputBufferSlots()[i].ItemData, Machine->GetInputBufferSlots()[i].Amount);
				}
			}
		}
		if (OutputSlot)
		{
			OutputSlot->OnSlotDropped.AddUniqueDynamic(this, &UFactoryMachineBasePanel::HandleSlotDrop);
			OutputSlot->InitSlotIdentity(Machine, false, 0);
			OutputSlot->UpdateSlotVisual(Machine->GetOutputBufferSlot().ItemData, Machine->GetOutputBufferSlot().Amount);
		}
		
		OnRecipeUpdated(Machine->GetCurrentRecipe());
	}
}

void UFactoryMachineBasePanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (OpenRecipeListPanelBtn)
	{
		OpenRecipeListPanelBtn->OnClicked.AddUniqueDynamic(this, &UFactoryMachineBasePanel::OnOpenRecipeListClicked);
	}
}

void UFactoryMachineBasePanel::NativeDestruct()
{
	if (AFactoryMachineBase* Machine = CachedMachine.Get())
	{
		Machine->OnInputBufferChanged.RemoveDynamic(this, &UFactoryMachineBasePanel::OnInputBufferUpdated);
		Machine->OnOutputBufferChanged.RemoveDynamic(this, &UFactoryMachineBasePanel::OnOutputBufferUpdated);
		Machine->OnCurrentRecipeChanged.RemoveDynamic(this, &UFactoryMachineBasePanel::OnRecipeUpdated);
	}
	
	Super::NativeDestruct();
}

void UFactoryMachineBasePanel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (AFactoryMachineBase* Machine = CachedMachine.Get())
	{
		if (CraftingProgressBar)
		{
			if (Machine->IsWorking() && CachedRecipe)
			{
				if (UFactoryCycleSubsystem* CycleSubsystem = GetWorld()->GetSubsystem<UFactoryCycleSubsystem>())
				{
					float MaxTime = CachedRecipe->ProcessingTime;
					if (MaxTime > 0)
					{
						int32 RemainingCycles = Machine->GetRemainingProductionCycles();
						float CycleInterval = CycleSubsystem->GetCycleInterval();
						float MaxTicks = FMath::RoundToFloat( MaxTime / CycleInterval);
						float CycleAlpha = CycleSubsystem->GetCycleAlpha();
					
						float ElapsedTicks = (MaxTicks - RemainingCycles) + CycleAlpha;
						float TargetPercent = FMath::Clamp(ElapsedTicks / MaxTicks, 0.f, 1.f);
					
						CraftingProgressBar->SetPercent(TargetPercent);
					}
					else
					{
						CraftingProgressBar->SetPercent(1.f);
					}
				}
			}
			else
			{
				// 작동중이 아니거나 레시피가 등록이 안된경우 진행률 0%로 표기
				CraftingProgressBar->SetPercent(0.f);
			}
		}
	}
}

void UFactoryMachineBasePanel::OnInputBufferUpdated(int32 SlotIndex, FFactorySlot SlotData)
{
	if (InputSlots.IsValidIndex(SlotIndex) && InputSlots[SlotIndex])
	{
		InputSlots[SlotIndex]->UpdateSlotVisual(SlotData.ItemData, SlotData.Amount);
	}
}

void UFactoryMachineBasePanel::OnOutputBufferUpdated(FFactorySlot SlotData)
{
	OutputSlot->UpdateSlotVisual(SlotData.ItemData, SlotData.Amount);
}

void UFactoryMachineBasePanel::OnRecipeUpdated(const UFactoryRecipeData* RecipeData)
{
	CachedRecipe = RecipeData;
}

void UFactoryMachineBasePanel::HandleSlotDrop(UFactoryFacilitySlotWidget* TargetSlot,
	const class UFactoryItemData* ItemData, int32 Amount, class UFactoryBaseSlotWidget* SourceWidget)
{
	AFactoryMachineBase* Machine = CachedMachine.Get();
	if (!Machine || !ItemData) return;
	
	if (UFactoryInventorySlotWidget* InventorySlotWidget = Cast<UFactoryInventorySlotWidget>(SourceWidget))
	{
		if (UFactoryInventoryComponent* InventoryComponent = InventorySlotWidget->GetInventoryComponent())
		{
			int32 FoundIndex;
			if (InputSlots.Find(TargetSlot, FoundIndex))
			{
				int32 RemainingAmount = 0;
				if (Machine->TryPutItemToBuffer(true, FoundIndex, ItemData, Amount, RemainingAmount))
				{
					// 성공적으로 들어간 개수만큼 인벤토리에서 제거
					int32 InsertedAmount = Amount - RemainingAmount;
					InventoryComponent->RemoveItemFromTargetSlot(InventorySlotWidget->GetSlotIndex(), InsertedAmount);
				}
			}
		}
	}
}

void UFactoryMachineBasePanel::OnOpenRecipeListClicked()
{
	if (RecipeListPopupWidgetBP && CachedMachine.IsValid())
	{
		if (UFactoryRecipeListPopupWidget* Popup = CreateWidget<UFactoryRecipeListPopupWidget>(
			GetWorld(), RecipeListPopupWidgetBP))
		{
			Popup->InitPopup(CachedMachine->GetAvailableRecipes());
            
			Popup->AddToViewport(100);
		}
	}
}
