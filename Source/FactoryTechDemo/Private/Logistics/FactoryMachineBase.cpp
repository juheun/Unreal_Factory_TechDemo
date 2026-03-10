// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryMachineBase.h"

#include "Items/FactoryItemVisual.h"
#include "Items/FactoryRecipeData.h"
#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Subsystems/FactoryDataSubsystem.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"


AFactoryMachineBase::AFactoryMachineBase()
{
	PrimaryActorTick.bCanEverTick = false;
}


void AFactoryMachineBase::BeginPlay()
{
	Super::BeginPlay();
	
	InitMachine();
}

void AFactoryMachineBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
	if (!WarehouseSubsystem) return;
	
	for (auto& InputBufferSlot : InputBufferSlots)
	{
		if (!InputBufferSlot.IsEmpty())
		{
			WarehouseSubsystem->AddItem(
				const_cast<UFactoryItemData*>(InputBufferSlot.ItemData.Get()), InputBufferSlot.Amount);
		}
	}
	
	if (!OutputBufferSlot.IsEmpty())
	{
		WarehouseSubsystem->AddItem(
			const_cast<UFactoryItemData*>(OutputBufferSlot.ItemData.Get()), OutputBufferSlot.Amount);
	}
}

void AFactoryMachineBase::InitMachine()
{
	// 인풋 버퍼 초기화
	InputBufferSlots.SetNum(InputBufferSize);
	for (int i = 0; i < InputBufferSlots.Num(); i++)
	{
		InputBufferSlots[i].Clear();
	}
	// 아웃풋 버퍼 초기화
	OutputBufferSlot.Clear();
	
	// 설비 아이덴티티 기반으로 이 설비에서 가공 가능한 레시피 데이터 가져오기
	UFactoryDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UFactoryDataSubsystem>();
	if (DataSubsystem && FacilityIdentity)
	{
		AvailableRecipes = DataSubsystem->GetRecipeDatasForFacility(FacilityIdentity);
	}
}

void AFactoryMachineBase::PlanCycle()
{
	// 가공 로직
	if (bIsWorking)
	{
		if (RemainingProductionCycle > 0)
		{
			RemainingProductionCycle--;
		}
		if (RemainingProductionCycle <= 0)
		{
			if (TryEndCraftItem())
			{
				bIsWorking = false;
			}
		}
	}
	
	// OutputPort로 아이템 밀어넣기 시도
	if (OutputBufferSlot.IsEmpty()) return;
	
	int32 MaxPorts = LogisticsOutputPortArr.Num();
	if (MaxPorts == 0) return;
	
	int32 OutputPortNum = OutputPortIndex;
	for (int i = 0; i < MaxPorts; i++)
	{
		if (OutputBufferSlot.IsEmpty()) break;
		
		int index = (OutputPortNum + i) % MaxPorts;
		if (!LogisticsOutputPortArr.IsValidIndex(index) || !LogisticsOutputPortArr[index]) continue;
		
		UFactoryInputPortComponent* TargetPort = LogisticsOutputPortArr[index]->GetConnectedInput();
		if (!TargetPort) continue;
		
		if (TargetPort->GetPortOwner()->CanPushItemFromBeforeObject(TargetPort))
		{
			UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
			if (!PoolSubsystem) return;
			
			FFactoryItemInstance NewInstance(OutputBufferSlot.ItemData);
			FVector SpawnLocation = LogisticsOutputPortArr[index]->GetComponentLocation();	// 현재 내 Output 포트 위치에 스폰
			FRotator SpawnRotation = LogisticsOutputPortArr[index]->GetComponentRotation();
			AFactoryItemVisual* ItemVisual = PoolSubsystem->GetItemFromPool<AFactoryItemVisual>(
				EFactoryPoolType::ItemVisual, SpawnLocation, SpawnRotation);
			ItemVisual->UpdateVisual(OutputBufferSlot.ItemData);
			if (ItemVisual)
			{
				NewInstance.VisualActor = ItemVisual;
			}

			// 완성된 인스턴스를 상대방 포트에 전달
			TargetPort->PendingItem = NewInstance;
			OutputPortIndex = (index + 1) % MaxPorts;
			OutputBufferSlot.Amount--;
			
			if (OutputBufferSlot.Amount <= 0)
			{
				OutputBufferSlot.Clear();
			}
		}
	}
}

void AFactoryMachineBase::ExecuteCycle()
{
	// InputPort에 Pending된 아이템이 있으면 InputBuffer에 가져옴
	int32 MaxPorts = LogisticsInputPortArr.Num();
	if (MaxPorts <= 0) return;
	
	int InputPortNum = InputPortIndex;
	for (int i = 0; i < MaxPorts; i++)
	{
		int index = (InputPortNum + i) % MaxPorts;
		
		if (!LogisticsInputPortArr.IsValidIndex(index) || !LogisticsInputPortArr[index]) continue;
		if (LogisticsInputPortArr[index]->PendingItem.IsValid())
		{
			if (PullItemFromInputPorts(LogisticsInputPortArr[index]->PendingItem))
			{
				if (AFactoryItemVisual* VisualActor = LogisticsInputPortArr[index]->PendingItem.VisualActor.Get())
				{
					UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
					if (!PoolSubsystem) return;
					
					PoolSubsystem->ReturnItemToPool(VisualActor);
				}
				LogisticsInputPortArr[index]->PendingItem = FFactoryItemInstance();
				InputPortIndex = (index + 1) % MaxPorts;
			}
		}
	}
	
	if (!bIsWorking)
	{
		TryCraftItem();
	}
}

void AFactoryMachineBase::UpdateView()
{
	// 애니메이션 재생 등
}

bool AFactoryMachineBase::CanPushItemFromBeforeObject(const UFactoryInputPortComponent* RequestPort) const
{
	if (!RequestPort || RequestPort->PendingItem.IsValid()) return false;	// Pending 되어 있지 않아야 밀어넣을 수 있음
	bool bHasEmptySlot = false;
	for (const FFactorySlot& Slot : InputBufferSlots)
	{
		if (Slot.ItemData == RequestPort->PendingItem.ItemData)
		{
			return !Slot.IsFull();
		}
		
		if (Slot.IsEmpty())
		{
			bHasEmptySlot = true;
		}
	}
	return bHasEmptySlot;
}

bool AFactoryMachineBase::PullItemFromInputPorts(FFactoryItemInstance& Item)
{
	if (!Item.IsValid()) return false;

	// 먼저 같은 아이템이 있는 덜 찬 슬롯을 찾음
	for (FFactorySlot& Slot : InputBufferSlots)
	{
		if (Slot.ItemData == Item.ItemData)
		{
			if (!Slot.IsFull())
			{
				Slot.Amount++;
				return true;
			}
			return false;
		}
	}
	// 없으면 완전히 빈 슬롯을 찾음
	for (FFactorySlot& Slot : InputBufferSlots)
	{
		if (Slot.IsEmpty())
		{
			Slot.ItemData = Item.ItemData;
			Slot.Amount = 1; // 벨트에서는 1개씩 들어옴
			return true;
		}
	}
	return false; // 버퍼 꽉 참	
}

bool AFactoryMachineBase::TryCraftItem()
{
	if (bIsWorking || AvailableRecipes.Num() == 0) return false;

	for (UFactoryRecipeData* Recipe : AvailableRecipes)
	{
		if (!Recipe) continue;
		
		bool bCanCraftThisRecipe = true;

		for (const FRecipeIngredient& Input : Recipe->Inputs)
		{
			bool bFoundIngredient = false;
			for (const FFactorySlot& Slot : InputBufferSlots)
			{
				if (Slot.ItemData == Input.ItemData)
				{
					bFoundIngredient = true;
					if (Input.Amount > Slot.Amount)
					{
						bCanCraftThisRecipe = false;
					}
					break;
				}
			}
			if (!bFoundIngredient || !bCanCraftThisRecipe)
			{
				bCanCraftThisRecipe = false;
				break;
			} 
		}

		if (bCanCraftThisRecipe)
		{
			// 아웃풋 버퍼에 공간이 있는지 미리 검사
			if (!OutputBufferSlot.IsEmpty() && 
				(OutputBufferSlot.ItemData != Recipe->Output.ItemData || 
				OutputBufferSlot.Amount + Recipe->Output.Amount > FFactorySlot::MaxCapacity))
			{
				return false; // 아웃풋 버퍼에 공간이 부족하면 가공 시작하지 않음
			}

			// 재료 소모
			for (const FRecipeIngredient& Input : Recipe->Inputs)
			{
				for (FFactorySlot& Slot : InputBufferSlots)
				{
					if (Slot.ItemData == Input.ItemData)
					{
						Slot.Amount -= Input.Amount;
						if (Slot.Amount <= 0) Slot.Clear();
					}
				}
			}

			// 가공 시작
			CurrentRecipe = Recipe;
			RemainingProductionCycle = Recipe->ProcessingTime; // 틱 단위 변환
			bIsWorking = true;
			return true;
		}
	}
	return false;
}

bool AFactoryMachineBase::TryEndCraftItem()
{
	if (CurrentRecipe)
	{
		// 이미 가공 시작 전에 공간 검사를 완료했으므로 바로 넣음
		OutputBufferSlot.ItemData = CurrentRecipe->Output.ItemData;
		OutputBufferSlot.Amount += CurrentRecipe->Output.Amount;
		
		CurrentRecipe = nullptr;
		return true;
	}
	
	return false;
}



