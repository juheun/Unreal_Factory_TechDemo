// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryMachineBase.h"

#include "Items/FactoryRecipeData.h"
#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Subsystems/FactoryCycleSubsystem.h"
#include "Subsystems/FactoryDataSubsystem.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"
#include "UI/World/FactoryPortBlockWarningComponent.h"
#include "UI/World/FactoryRecipeBillboardComponent.h"
#include "UI/World/FactorySmartNameplateComponent.h"
#include "UI/World/UFactoryFacilityBlockWarningComponent.h"


AFactoryMachineBase::AFactoryMachineBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SmartNameplateComponent = CreateDefaultSubobject<UFactorySmartNameplateComponent>(TEXT("SartNameplateComponent"));
	SmartNameplateComponent->SetupAttachment(RootComponent);
	
	RecipeBillboardComponent = CreateDefaultSubobject<UFactoryRecipeBillboardComponent>(TEXT("RecipeBillboardComponent"));
	RecipeBillboardComponent->SetupAttachment(RootComponent);
	
	FacilityBlockWarningComponent = CreateDefaultSubobject<UFactoryFacilityBlockWarningComponent>(TEXT("FacilityBlockWarningComponent"));
	FacilityBlockWarningComponent->SetupAttachment(RootComponent);
	
	LogisticsObjectType = ELogisticsObjectType::Facility;
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
			WarehouseSubsystem->AddItem(InputBufferSlot.ItemData, InputBufferSlot.Amount);
		}
	}
	
	if (!OutputBufferSlot.IsEmpty())
	{
		WarehouseSubsystem->AddItem(OutputBufferSlot.ItemData, OutputBufferSlot.Amount);
	}
}

void AFactoryMachineBase::InitObject(const UFactoryObjectData* Data)
{
	Super::InitObject(Data);
	
	// 설비 이름 표시
	if (SmartNameplateComponent)
	{
		SmartNameplateComponent->InitNameplate(Data);
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
	
	if (RecipeBillboardComponent)
	{
		OnCurrentRecipeChanged.AddDynamic(RecipeBillboardComponent, &UFactoryRecipeBillboardComponent::OnRecipeChangedCallback);
		RecipeBillboardComponent->OnRecipeChangedCallback(CurrentRecipe);
	}
	
	if (FacilityBlockWarningComponent)
	{
		OnFacilityBlockedStateChanged.AddDynamic(FacilityBlockWarningComponent, &UFactoryFacilityBlockWarningComponent::OnFacilityBlockCallback);
		FacilityBlockWarningComponent->OnFacilityBlockCallback(bIsMachineBlockedOnCycle);
	}
	
	for (auto& InputPort : LogisticsInputPortArr)
	{
		UFactoryPortBlockWarningComponent* PortBlockWarningComponent = NewObject<UFactoryPortBlockWarningComponent>(this);
		PortBlockWarningComponent->SetupAttachment(InputPort);
		float OffsetX = InputPort->GetScaledBoxExtent().X + 1.f;
		PortBlockWarningComponent->AddRelativeLocation(FVector(-OffsetX, 0.f, 0.f)); // 설비에 묻히지 않게 조금 이동
		PortBlockWarningComponent->RegisterComponent();
		InputPort->OnPortBlockedStateChanged.AddDynamic(PortBlockWarningComponent, &UFactoryPortBlockWarningComponent::OnPortBlockedCallback);
	}
	for (auto& OutputPort : LogisticsOutputPortArr)
	{
		UFactoryPortBlockWarningComponent* PortBlockWarningComponent = NewObject<UFactoryPortBlockWarningComponent>(this);
		PortBlockWarningComponent->SetupAttachment(OutputPort);
		float OffsetX = OutputPort->GetScaledBoxExtent().X + 1.f;
		PortBlockWarningComponent->AddRelativeLocation(FVector(OffsetX, 0.f, 0.f)); // 설비에 묻히지 않게 조금 이동
		PortBlockWarningComponent->RegisterComponent();
		OutputPort->OnPortBlockedStateChanged.AddDynamic(PortBlockWarningComponent, &UFactoryPortBlockWarningComponent::OnPortBlockedCallback);
	}
}

void AFactoryMachineBase::PlanCycle()
{
	bIsMachineBlockedOnCycle = false;
	
	// OutputPort로 아이템 밀어넣기 시도
	TryPushOutputToPorts();
	
	// 가공 로직
	if (bIsWorking)
	{
		if (UFactoryCycleSubsystem* CycleSubsystem = GetWorld()->GetSubsystem<UFactoryCycleSubsystem>())
		{
			if (RemainingProductionCycleTime > 0)
			{
				RemainingProductionCycleTime -= CycleSubsystem->GetCycleInterval();
			}
			if (RemainingProductionCycleTime <= 0)
			{
				if (TryEndCraftItem())
				{
					bIsWorking = false;
				}
				else
				{
					bIsMachineBlockedOnCycle = true;
				}
			}
		}
	}
}

void AFactoryMachineBase::LatePlanCycle()
{
	// 아웃풋 버퍼 밀어내기 재시도
	if (!OutputBufferSlot.IsEmpty())
	{
		TryPushOutputToPorts();
	}

	// 가공 완료 재시도
	if (bIsMachineBlockedOnCycle && bIsWorking && RemainingProductionCycleTime <= 0)
	{
		if (TryEndCraftItem())
		{
			bIsWorking = false;
			bIsMachineBlockedOnCycle = false;
		}
	}

	// 가공이 완료되었거나, 원래 쉬고 있었다면 즉시 새 가공을 시작합
	if (!bIsWorking)
	{
		TryCraftItem(); 
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
		int Index = (InputPortNum + i) % MaxPorts;
		
		if (!LogisticsInputPortArr.IsValidIndex(Index) || !LogisticsInputPortArr[Index]) continue;
		if (LogisticsInputPortArr[Index]->PendingItem.IsValid())
		{
			bool bIsPulled = PullItemFromInputPorts(LogisticsInputPortArr[Index]->PendingItem);
			
			if (!bIsPulled)
			{
				if (UFactoryWarehouseSubsystem* Warehouse = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>())
				{
					Warehouse->AddItem(LogisticsInputPortArr[Index]->PendingItem.ItemData, 1);
				}
			}

			LogisticsInputPortArr[Index]->PendingItem = FFactoryItemInstance();
			InputPortIndex = (Index + 1) % MaxPorts;
		}
	}
	
	if (!bIsWorking)
	{
		TryCraftItem();
	}
	
	// 인풋 버퍼가 공간이 있다면 모든 인풋 포트의 막힘 상태를 풀어줘야함
	bool bHasEmptySpace = false;
	for (const FFactorySlot& Slot : InputBufferSlots) {
		if (!Slot.IsFull()) { bHasEmptySpace = true; break; }
	}
	if (bHasEmptySpace) {
		for (auto& Port : LogisticsInputPortArr) {
			if (Port) Port->SetPortBlocked(false);
		}
	}
}

void AFactoryMachineBase::UpdateView()
{
	// 애니메이션 재생 등
	
	// 가공 중도 아니고, 남은 재료로 새 가공을 시작할 수도 없는 상태라면 경고
	bool bIsStarvingOrNoRecipe = !bIsWorking && AvailableRecipes.Num() == 0; 
    
	// (막혔거나 OR 레시피가 없거나) 둘 중 하나면 막힘 방송
	SetFacilityBlocked(bIsMachineBlockedOnCycle || bIsStarvingOrNoRecipe);
}

bool AFactoryMachineBase::CanPushItemFromBeforeObject(
	UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem)
{
	if (!RequestPort || RequestPort->PendingItem.IsValid()) return false;	// Pending 되어 있지 않아야 밀어넣을 수 있음
	if (!IncomingItem) return false;
	
	for (const FFactorySlot& Slot : InputBufferSlots)
	{
		if (Slot.ItemData == IncomingItem)
		{
			RequestPort->SetPortBlocked(Slot.IsFull());
			return !Slot.IsFull();
		}
	}
	
	for (const FFactorySlot& Slot : InputBufferSlots)
	{
		if (Slot.IsEmpty())
		{
			RequestPort->SetPortBlocked(false);
			return true;
		}
	}
	RequestPort->SetPortBlocked(true);
	return false;
}

bool AFactoryMachineBase::PullItemFromInputPorts(FFactoryItemInstance& Item)
{
	if (!Item.IsValid()) return false;

	// 먼저 같은 아이템이 있는 덜 찬 슬롯을 찾음
	for (int32 i = 0; i < InputBufferSlots.Num(); ++i)
	{
		if (InputBufferSlots[i].ItemData == Item.ItemData)
		{
			if (!InputBufferSlots[i].IsFull())
			{
				InputBufferSlots[i].Amount++;
				OnInputBufferChanged.Broadcast(i, InputBufferSlots[i]);
				return true;
			}
			return false;
		}
	}
	// 없으면 완전히 빈 슬롯을 찾음
	for (int32 i = 0; i < InputBufferSlots.Num(); ++i)
	{
		if (InputBufferSlots[i].IsEmpty())
		{
			InputBufferSlots[i].ItemData = Item.ItemData;
			InputBufferSlots[i].Amount = 1; // 벨트에서는 1개씩 들어옴
			OnInputBufferChanged.Broadcast(i, InputBufferSlots[i]);
			return true;
		}
	}
	return false; // 버퍼 꽉 참	
}

void AFactoryMachineBase::TryPushOutputToPorts()
{
	for (auto& Port : LogisticsOutputPortArr)
	{
		if (Port) Port->SetPortBlocked(false);
	}
	
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
		if (!TargetPort || TargetPort->PendingItem.IsValid()) continue;
		
		if (TargetPort->GetPortOwner()->CanPushItemFromBeforeObject(TargetPort, OutputBufferSlot.ItemData))
		{
			UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
			if (!PoolSubsystem) return;
			
			FFactoryItemInstance NewInstance(OutputBufferSlot.ItemData);
			// 완성된 인스턴스를 상대방 포트에 전달
			TargetPort->PendingItem = NewInstance;
			OutputPortIndex = (index + 1) % MaxPorts;
			OutputBufferSlot.Amount--;
			
			if (OutputBufferSlot.Amount <= 0)
			{
				OutputBufferSlot.Clear();
			}
			
			LogisticsOutputPortArr[index]->SetPortBlocked(false);
		}
		else
		{
			LogisticsOutputPortArr[index]->SetPortBlocked(true);
		}
	}
}

bool AFactoryMachineBase::TryCraftItem()
{
	if (bIsWorking || AvailableRecipes.Num() == 0) return false;
	
	for (const FFactorySlot& Slot : InputBufferSlots)
	{
		if (Slot.IsEmpty())
		{
			return false;	// 기획상 레시피 재료는 설비 슬롯 수와 일치. 아직 슬롯이 채워지지 않으면 가공 시작 불가
		}
	}
	
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
				bIsMachineBlockedOnCycle = true;		// 버퍼가 막힌것은 설비가 막힌것
				return false; // 아웃풋 버퍼에 공간이 부족하면 가공 시작하지 않음
			}

			// 재료 소모
			for (const FRecipeIngredient& Input : Recipe->Inputs)
			{
				for (int32 i = 0; i < InputBufferSlots.Num(); ++i)
				{
					if (InputBufferSlots[i].ItemData == Input.ItemData)
					{
						InputBufferSlots[i].Amount -= Input.Amount;
            
						if (InputBufferSlots[i].Amount <= 0)
						{
							// 패널 슬롯에서 아이템이 0개라도 아이콘을 표시하기위해 clear하지 않음ㅁㅁ
							InputBufferSlots[i].Amount = 0;
						}

						OnInputBufferChanged.Broadcast(i, InputBufferSlots[i]);
					}
				}
			}

			// 가공 시작
			CurrentRecipe = Recipe;
			RemainingProductionCycleTime = Recipe->ProcessingTime; // 틱 단위 변환
			OnCurrentRecipeChanged.Broadcast(CurrentRecipe);
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
		if ((OutputBufferSlot.IsEmpty() || OutputBufferSlot.ItemData == CurrentRecipe->Output.ItemData)
			&& OutputBufferSlot.Amount + CurrentRecipe->Output.Amount <= FFactorySlot::MaxCapacity)
		{
			// 이미 가공 시작 전에 공간 검사를 완료했으므로 바로 넣음
			OutputBufferSlot.ItemData = CurrentRecipe->Output.ItemData;
			OutputBufferSlot.Amount += CurrentRecipe->Output.Amount;
			OnOutputBufferChanged.Broadcast(OutputBufferSlot);
			return true;
		}
	}
	
	return false;
}


bool AFactoryMachineBase::TryPutItemToBuffer(bool bIsInputBuffer, int32 SlotIndex, const UFactoryItemData* ItemData,
	int32 AmountToPut, int32& OutRemainingAmount)
{
	OutRemainingAmount = AmountToPut;
	if (!ItemData || AmountToPut <= 0) return false;

	if (bIsInputBuffer)
	{
		if (!InputBufferSlots.IsValidIndex(SlotIndex)) return false;

		FFactorySlot& TargetSlot = InputBufferSlots[SlotIndex];

		// 빈 슬롯에 넣으려는 경우 다른 슬롯에 이미 같은 아이템이 있는지 검사 (제약 조건 준수)
		if (TargetSlot.IsEmpty())
		{
			for (int32 i = 0; i < InputBufferSlots.Num(); ++i)
			{
				if (i != SlotIndex && InputBufferSlots[i].ItemData == ItemData)
				{
					// 이미 다른 슬롯이 이 아이템을 전담하고 있으므로, 새로운 슬롯에 넣는 것을 거부
					return false; 
				}
			}
		}

		// 해당 슬롯이 비어있거나, 넣으려는 아이템과 같은 종류일 때만 진행
		if (TargetSlot.IsEmpty() || TargetSlot.ItemData == ItemData)
		{
			int32 AvailableSpace = FFactorySlot::MaxCapacity - TargetSlot.Amount;
            
			int32 AmountToInsert = FMath::Min(AmountToPut, AvailableSpace);

			if (AmountToInsert > 0)
			{
				TargetSlot.ItemData = ItemData;
				TargetSlot.Amount += AmountToInsert;
				OutRemainingAmount -= AmountToInsert; // 다 못 넣었으면 남은 개수가 담김

				OnInputBufferChanged.Broadcast(SlotIndex, TargetSlot);
				return true;
			}
		}
	}
	else
	{
		// 아웃풋 슬롯에는 플레이어가 인벤토리에서 수동으로 아이템을 집어넣지 못하게 막습니다.
		return false;
	}

	return false;
}

bool AFactoryMachineBase::TryTakeItemFromBuffer(bool bIsInputBuffer, int32 SlotIndex, int32 AmountToTake,
	FFactorySlot& OutTakenSlot)
{
	OutTakenSlot.Clear();
	if (AmountToTake <= 0) return false;

	if (bIsInputBuffer)
	{
		// 인풋 버퍼에서 빼기
		if (!InputBufferSlots.IsValidIndex(SlotIndex)) return false;
		FFactorySlot& TargetSlot = InputBufferSlots[SlotIndex];

		if (!TargetSlot.IsEmpty())
		{
			int32 ActuallyTaken = FMath::Min(AmountToTake, TargetSlot.Amount);
			OutTakenSlot.ItemData = TargetSlot.ItemData;
			OutTakenSlot.Amount = ActuallyTaken;

			TargetSlot.Amount -= ActuallyTaken;
			// 패널 슬롯에서 아이템이 0개라도 아이콘을 표시하기위해 clear하지 않음
			if (TargetSlot.Amount <= 0) TargetSlot.Amount = 0;

			OnInputBufferChanged.Broadcast(SlotIndex, TargetSlot);
			return true;
		}
	}
	else
	{
		// 아웃풋 버퍼에서 빼기 (SlotIndex는 무시하고 단일 버퍼 사용)
		if (!OutputBufferSlot.IsEmpty())
		{
			int32 ActuallyTaken = FMath::Min(AmountToTake, OutputBufferSlot.Amount);
			OutTakenSlot.ItemData = OutputBufferSlot.ItemData;
			OutTakenSlot.Amount = ActuallyTaken;

			OutputBufferSlot.Amount -= ActuallyTaken;
			if (OutputBufferSlot.Amount <= 0) OutputBufferSlot.Clear();

			OnOutputBufferChanged.Broadcast(OutputBufferSlot);
			return true;
		}
	}

	return false;
}



