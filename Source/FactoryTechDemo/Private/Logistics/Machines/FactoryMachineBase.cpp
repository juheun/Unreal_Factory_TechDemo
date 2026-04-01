// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/Machines/FactoryMachineBase.h"

#include "Items/FactoryRecipeData.h"
#include "Logistics/Ports/FactoryInputPortComponent.h"
#include "Logistics/Ports/FactoryOutputPortComponent.h"
#include "Subsystems/FactoryCycleSubsystem.h"
#include "Subsystems/FactoryDataSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"
#include "UI/World/Components/FactoryPortBlockWarningComponent.h"
#include "UI/World/Components/FactoryRecipeBillboardComponent.h"
#include "UI/World/Components/FactorySmartNameplateComponent.h"
#include "UI/World/Components/UFactoryFacilityBlockWarningComponent.h"


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
	// 아웃풋 포트 개수만큼 배열 false로 초기화
	OutputPortBlockedStates.Init(false, LogisticsOutputPortArr.Num());
	OutputPortPulledThisCycle.Init(false, LogisticsOutputPortArr.Num());
	InputPortBlockedStates.Init(false, LogisticsInputPortArr.Num());
	
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

void AFactoryMachineBase::InitPhase()
{
	bIsMachineBlockedOnCycle = false;
	
	// 상태 초기화
	if (OutputPortPulledThisCycle.Num() != LogisticsOutputPortArr.Num())
	{
		OutputPortBlockedStates.Init(false, LogisticsOutputPortArr.Num());
		OutputPortPulledThisCycle.Init(false, LogisticsOutputPortArr.Num());
	}
	else
	{
		for (int i = 0; i < OutputPortPulledThisCycle.Num(); i++)
		{
			OutputPortBlockedStates[i] = false;
			OutputPortPulledThisCycle[i] = false;
		}
	}
	if (InputPortBlockedStates.Num() != LogisticsInputPortArr.Num())
	{
		InputPortBlockedStates.Init(false, LogisticsInputPortArr.Num());
	}
	else
	{
		for (int i = 0; i < InputPortBlockedStates.Num(); i++)
		{
			InputPortBlockedStates[i] = false;
		}
	}	
}

void AFactoryMachineBase::LogisticsPhase()
{
	TryPushOutputToPorts();
	TryPullInputFromPorts();
}

void AFactoryMachineBase::LateLogisticsPhase()
{
	TryPushOutputToPorts();
	TryPullInputFromPorts();
}

void AFactoryMachineBase::LogicPhase()
{
	// 가공 로직
	if (bIsWorking)
	{
		if (RemainingProductionCycles > 0)
		{
			RemainingProductionCycles -= 1;
		}
		if (RemainingProductionCycles <= 0)
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
	if (!bIsWorking)
	{
		TryCraftItem();
	}
}

void AFactoryMachineBase::VisualPhase()
{
	// 애니메이션 재생 등
	
	// 가공 중도 아니고, 남은 재료로 새 가공을 시작할 수도 없는 상태라면 경고
	bool bIsStarvingOrNoRecipe = !bIsWorking && AvailableRecipes.Num() == 0; 
	// (막혔거나 OR 레시피가 없거나) 둘 중 하나면 막힘 방송
	SetFacilityBlocked(bIsMachineBlockedOnCycle || bIsStarvingOrNoRecipe);
	
	// 아웃포트 막힘 상태 방송
	for (int i = 0; i < LogisticsOutputPortArr.Num(); i++)
	{
		if (OutputBufferSlot.IsEmpty()) 
		{
			OutputPortBlockedStates[i] = false;
		}
		if (LogisticsOutputPortArr[i])
		{
			LogisticsOutputPortArr[i]->SetPortBlocked(OutputPortBlockedStates[i]);
		}
	}
	// 인풋포트 막힘 상태 방송
	for (int i = 0; i < LogisticsInputPortArr.Num(); i++)
	{
		if (LogisticsInputPortArr[i])
		{
			LogisticsInputPortArr[i]->SetPortBlocked(InputPortBlockedStates[i]);
		}
	}
}

void AFactoryMachineBase::TryPullInputFromPorts()
{
	int32 MaxPorts = LogisticsInputPortArr.Num();
	if (MaxPorts <= 0) return;
	
	int InputPortNum = InputPortIndex;
	for (int i = 0; i < MaxPorts; i++)
	{
		int Index = (InputPortNum + i) % MaxPorts;
		UFactoryInputPortComponent* InputPort = LogisticsInputPortArr[Index];
		if (!InputPort) continue;
		
		UFactoryOutputPortComponent* ConnectedOutputPort = InputPort->GetConnectedOutput();
		if (!ConnectedOutputPort) continue;
		
		AFactoryLogisticsObjectBase* PrevObj = ConnectedOutputPort->GetPortOwner();
		if (!PrevObj) continue;
		
		// 1. 이전 설비 아이템 구경하기
		const UFactoryItemData* PeekedItem = PrevObj->PeekOutputItem(ConnectedOutputPort);
		if (!PeekedItem)
		{
			InputPortBlockedStates[Index] = false; 
			continue;
		}
		
		// 받을 수 있는지 판단
		if (CanReceiveItem(InputPort, PeekedItem))
		{
			FFactoryItemInstance PulledItem = PrevObj->ConsumeItem(ConnectedOutputPort);
			ReceiveItem(InputPort, PulledItem);
			InputPortBlockedStates[Index] = false;
			InputPortIndex = (Index + 1) % MaxPorts; // 라운드 로빈 갱신
		}
		else
		{
			InputPortBlockedStates[Index] = true; // 꽉 차서 혹은 못 받는 아이템이라 막힘
		}
	}
}

void AFactoryMachineBase::TryPushOutputToPorts()
{
	// 내 아웃풋 버퍼에 템이 없으면 밀어낼 것도 없음
	if (OutputBufferSlot.IsEmpty()) return;
	
	int32 MaxOutputs = LogisticsOutputPortArr.Num();
	if (MaxOutputs <= 0) return;
	
	int32 StartIndex = OutputPortIndex;
	for (int i = 0; i < MaxOutputs; i++)
	{
		if (OutputBufferSlot.IsEmpty()) break;
		
		int32 Index = (StartIndex + i) % MaxOutputs;
		if (OutputPortPulledThisCycle[Index]) continue;
		UFactoryOutputPortComponent* OutPort = LogisticsOutputPortArr[Index];
		if (!OutPort) continue;
		
		UFactoryInputPortComponent* TargetIn = OutPort->GetConnectedInput();
		if (!TargetIn) continue;
		
		AFactoryLogisticsObjectBase* TargetObj = TargetIn->GetPortOwner();
		if (!TargetObj) continue;
		
		if (TargetObj->CanReceiveItem(TargetIn, OutputBufferSlot.ItemData))
		{
			// 상대방 벨트에 강제 주입
			FFactoryItemInstance PushedItem(OutputBufferSlot.ItemData);
			TargetObj->ForceAcceptPushedItem(TargetIn, PushedItem); 
			
			OutputBufferSlot.Amount--;
			if (OutputBufferSlot.Amount <= 0) OutputBufferSlot.Clear();
			
			// UI 갱신 및 라운드 로빈 포인터 이동
			OutputPortPulledThisCycle[Index] = true; // 이름은 Pulled지만 내가 밀어넣었음
			OutputPortBlockedStates[Index] = false;
			OutputPortIndex = (Index + 1) % MaxOutputs; 
		}
		else
		{
			OutputPortBlockedStates[Index] = true;
		}
	}
}

bool AFactoryMachineBase::CanReceiveItem(UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem)
{
	if (!IncomingItem) return false;
	
	for (const FFactorySlot& Slot : InputBufferSlots)
	{
		if (Slot.ItemData == IncomingItem)
		{
			return !Slot.IsFull();
		}
	}
	
	for (const FFactorySlot& Slot : InputBufferSlots)
	{
		if (Slot.IsEmpty())
		{
			return true;
		}
	}
	return false;
}

const UFactoryItemData* AFactoryMachineBase::PeekOutputItem(UFactoryOutputPortComponent* RequestPort)
{
	return nullptr;		// 머신은 능동적으로 다음 설비로 아이템을 밀어냄
}

FFactoryItemInstance AFactoryMachineBase::ConsumeItem(UFactoryOutputPortComponent* RequestPort)
{
	return FFactoryItemInstance();		// 머신은 능동적으로 다음 설비로 아이템을 밀어냄
}

void AFactoryMachineBase::ReceiveItem(UFactoryInputPortComponent* RequestPort, FFactoryItemInstance Item)
{
	if (!Item.IsValid()) return;

	// 먼저 같은 아이템이 있는 덜 찬 슬롯을 찾음
	for (int32 i = 0; i < InputBufferSlots.Num(); ++i)
	{
		if (InputBufferSlots[i].ItemData == Item.ItemData)
		{
			if (!InputBufferSlots[i].IsFull())
			{
				InputBufferSlots[i].Amount++;
				OnInputBufferChanged.Broadcast(i, InputBufferSlots[i]);
				return;
			}
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
			return;
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
			if (UFactoryCycleSubsystem* CycleSubsystem = GetWorld()->GetSubsystem<UFactoryCycleSubsystem>())
			{
				float Interval = CycleSubsystem->GetCycleInterval();
				RemainingProductionCycles = FMath::RoundToInt(Recipe->ProcessingTime / Interval); // 틱 단위 변환
			}
			CurrentRecipe = Recipe;
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



