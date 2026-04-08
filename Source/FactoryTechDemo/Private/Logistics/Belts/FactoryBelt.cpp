// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/Belts/FactoryBelt.h"

#include "Components/SplineComponent.h"
#include "Logistics/Ports/FactoryOutputPortComponent.h"
#include "Logistics/Ports/FactoryInputPortComponent.h"
#include "Logistics/FactoryLogisticsTypes.h"
#include "Items/FactoryItemData.h"
#include "Placement/Components/FactoryPlacementComponent.h"
#include "Core/FactoryDeveloperSettings.h"
#include "Subsystems/FactoryItemRenderSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"


AFactoryBelt::AFactoryBelt()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	SplineComponent = CreateDefaultSubobject<USplineComponent>("SplineComponent");
	SplineComponent->SetupAttachment(RootComponent);
	
	LogisticsObjectType = ELogisticsObjectType::Conveyor;
}

void AFactoryBelt::BeginPlay()
{
	Super::BeginPlay();
	
	CacheSplineTransforms();
}

void AFactoryBelt::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (UFactoryItemRenderSubsystem* RenderSubsystem = GetWorld()->GetSubsystem<UFactoryItemRenderSubsystem>())
	{
		RenderSubsystem->UnregisterActiveBelt(this);
	}
	
	UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
	if (!WarehouseSubsystem) return;

	if (CurrentItem.IsValid())
	{
		WarehouseSubsystem->AddItem(CurrentItem.ItemData.Get(), 1);
	}
}

void AFactoryBelt::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetBeltType(BeltType);
}

#pragma region Belt Logic

void AFactoryBelt::InitPhase()
{
	bReceivedThisCycle = false;
}

void AFactoryBelt::LogisticsPhase()
{
	TryPullInputFromPorts();
}

void AFactoryBelt::LateLogisticsPhase()
{
	if (!CurrentItem.IsValid())
	{
		TryPullInputFromPorts();
	}
}

void AFactoryBelt::LogicPhase()
{
}

void AFactoryBelt::VisualPhase()
{
	if (bIsBeltRegistered != CurrentItem.IsValid())
	{
		if (UFactoryItemRenderSubsystem* RenderSubsystem = GetWorld()->GetSubsystem<UFactoryItemRenderSubsystem>())
		{
			bIsBeltRegistered = CurrentItem.IsValid();
			if (bIsBeltRegistered)
			{
				RenderSubsystem->RegisterActiveBelt(this);	
			}
			else
			{
				RenderSubsystem->UnregisterActiveBelt(this);
			}
		}
	}
}

void AFactoryBelt::TryPullInputFromPorts()
{
	if (CurrentItem.IsValid())	//CurrentItem이 비어있지 않다는건 다음 설비가 가져가지않았고, 막혔다는뜻
	{
		bIsBeltStop = true;
		return;
	}
	
	if (!LogisticsInputPortArr.IsValidIndex(0) || !LogisticsInputPortArr[0]) return;
	
	UFactoryInputPortComponent* InputPort = LogisticsInputPortArr[0];
	UFactoryOutputPortComponent* ConnectedOut = InputPort->GetConnectedOutput();
	if (!ConnectedOut) return;
	AFactoryLogisticsObjectBase* PrevObj = ConnectedOut->GetPortOwner();
	if (!PrevObj) return;
	
	const UFactoryItemData* PeekedItem = PrevObj->PeekOutputItem(ConnectedOut);
	if (PeekedItem && CanReceiveItem(InputPort, PeekedItem))
	{
		FFactoryItemInstance PulledItem = PrevObj->ConsumeItem(ConnectedOut);
		ReceiveItem(InputPort, PulledItem);
	}
}

const UFactoryItemData* AFactoryBelt::PeekOutputItem(UFactoryOutputPortComponent* RequestPort)
{
	if (bReceivedThisCycle) return nullptr;
	return CurrentItem.ItemData;
}

FFactoryItemInstance AFactoryBelt::ConsumeItem(UFactoryOutputPortComponent* RequestPort)
{
	if (bReceivedThisCycle) return FFactoryItemInstance();
	
	FFactoryItemInstance InstanceToGive = CurrentItem;
	CurrentItem = FFactoryItemInstance();
	return InstanceToGive;
}

bool AFactoryBelt::CanReceiveItem(UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem)
{
	if (!IncomingItem) return false;
	return !CurrentItem.IsValid();
}

void AFactoryBelt::ReceiveItem(UFactoryInputPortComponent* RequestPort, FFactoryItemInstance Item)
{
	if (!Item.IsValid()) return;
	CurrentItem = Item;
	bIsBeltStop = false;
	
	bReceivedThisCycle = true;
}

void AFactoryBelt::SetBeltType(EBeltType Type)
{
	// 벨트 유형이 바뀌며 유령연결이 되는것 방지
	if (LogisticsOutputPortArr.IsValidIndex(0) && LogisticsOutputPortArr[0])
	{
		LogisticsOutputPortArr[0]->Disconnect();
	}
	if (LogisticsInputPortArr.IsValidIndex(0) && LogisticsInputPortArr[0])
	{
		LogisticsInputPortArr[0]->Disconnect();
	}
	
	BeltType = Type;
	UpdateSplinePath(Type);
	UpdateBeltVisual(Type);
	
	if (HasActorBegunPlay())
	{
		if (LogisticsOutputPortArr.IsValidIndex(0) && LogisticsOutputPortArr[0])
		{
			LogisticsOutputPortArr[0]->ForceScanConnection();
		}
		if (LogisticsInputPortArr.IsValidIndex(0) && LogisticsInputPortArr[0])
		{
			LogisticsInputPortArr[0]->ForceScanConnection();
		}
	}
}

FVector AFactoryBelt::GetBeltExitDirection() const
{
	if (!LogisticsOutputPortArr.IsValidIndex(0) || !LogisticsOutputPortArr[0]) 
		return FVector::ZeroVector;
	
	return LogisticsOutputPortArr[0]->GetForwardVector().GetSafeNormal();
}

void AFactoryBelt::UpdateSplinePath(EBeltType Type)
{
	if (!SplineComponent) return;
	SplineComponent->ClearSplinePoints(true);
	
	// 스트레스 툴 등으로 자동적으로 벨트를 생성한경우 LogisticsOutputPortArr가 비어있으므로 강제로 한번 초기화
	if (LogisticsOutputPortArr.IsEmpty() || LogisticsInputPortArr.IsEmpty())
	{
		InitializeLogisticsPort();
	}
	
	float GridLength = GetDefault<UFactoryDeveloperSettings>()->GetGridLength();
	float HalfGridLength = GridLength * 0.5f;
	float CurveStrength = HalfGridLength; // 곡률의 부드러움 결정 (보통 변의 길이의 절반 정도)
	
	// 시작점 (Input Port 위치)
	FVector StartPos = FVector(-HalfGridLength, 0.0f, BeltHeight); // 벨트 뒤쪽 끝
	FVector StartTangent = FVector(CurveStrength * 2.0f, 0.0f, 0.0f); // 앞을 향하는 힘

	// 끝점 (Output Port 위치)
	FVector EndPos;
	FVector EndTangent;
	
	switch (Type)
	{
	default:
	case EBeltType::Straight:
		EndPos = FVector(HalfGridLength, 0.0f, BeltHeight);
		EndTangent = FVector(CurveStrength * 2.0f, 0.0f, 0.0f);
		break;

	case EBeltType::LeftTurn:
		EndPos = FVector(0.0f, -HalfGridLength, BeltHeight); // 왼쪽으로 90도
		EndTangent = FVector(0.0f, -CurveStrength * 2.0f, 0.0f);
		break;

	case EBeltType::RightTurn:
		EndPos = FVector(0.0f, HalfGridLength, BeltHeight); // 오른쪽으로 90도
		EndTangent = FVector(0.0f, CurveStrength * 2.0f, 0.0f);
		break;
	}
	
	if (LogisticsOutputPortArr.IsValidIndex(0) && LogisticsOutputPortArr[0])
	{
		// 타입에 따른 포트 위치,회전값 설정
		float TargetYaw = 0.f;
		if (Type == EBeltType::LeftTurn) TargetYaw = -90.f;
		else if (Type == EBeltType::RightTurn) TargetYaw = 90.f;
		
		FVector OutPutPort = EndPos;
		OutPutPort.Z = BeltHeight * 0.5f;
		
		float PortOffset = LogisticsOutputPortArr[0]->GetScaledBoxExtent().X;
		FVector ForwardDir = FRotator(0.f, TargetYaw, 0.f).Vector();
		OutPutPort -= ForwardDir * PortOffset;
		LogisticsOutputPortArr[0]->SetRelativeLocation(OutPutPort);
		LogisticsOutputPortArr[0]->SetRelativeRotation(FRotator(0, TargetYaw, 0));
	}

	// 스플라인 포인트 추가 (로컬 좌표계 기준)
	SplineComponent->AddSplinePoint(StartPos, ESplineCoordinateSpace::Local, true);
	SplineComponent->SetTangentAtSplinePoint(0, StartTangent, ESplineCoordinateSpace::Local, true);

	SplineComponent->AddSplinePoint(EndPos, ESplineCoordinateSpace::Local, true);
	SplineComponent->SetTangentAtSplinePoint(1, EndTangent, ESplineCoordinateSpace::Local, true);

	// 베지어 곡선으로 부드럽게 만들기 위해 포인트 타입 설정
	SplineComponent->SetSplinePointType(0, ESplinePointType::CurveCustomTangent, true);
	SplineComponent->SetSplinePointType(1, ESplinePointType::CurveCustomTangent, true);
	
	SplineComponent->UpdateSpline(); 
	SplineComponent->bSplineHasBeenEdited = true;
	
	TotalSpineLength = SplineComponent->GetSplineLength();
	
	if (!IsTemplate())
	{
		CacheSplineTransforms();
	}
}

void AFactoryBelt::UpdateBeltVisual(EBeltType Type)
{
	if (!MeshComponent) return;
	TObjectPtr<UStaticMesh>* FoundMesh = BeltMeshMap.Find(Type);
	
	if (FoundMesh && *FoundMesh)
	{
		MeshComponent->SetStaticMesh(BeltMeshMap[Type]);
	}
}

FTransform AFactoryBelt::GetSpineDistance(float Alpha) const
{
	// 캐싱된 데이터가 없으면 기본값 반환
	if (CachedSplineTransforms.IsEmpty()) return FTransform::Identity;
    
	// Alpha가 범위를 벗어날 경우
	if (Alpha <= 0.0f) return CachedSplineTransforms[0];
	if (Alpha >= 1.0f) return CachedSplineTransforms.Last();

	// Alpha값을 배열의 인덱스로 변환 (예: Alpha 0.25면 FloatIndex는 2.5)
	float FloatIndex = Alpha * (CachedSplineTransforms.Num() - 1);
    
	// 사이를 지나는 두 점의 인덱스를 구함 (예: 2와 3)
	int32 Index0 = FMath::FloorToInt(FloatIndex);
	int32 Index1 = FMath::CeilToInt(FloatIndex);
    
	// 두 점 사이의 소수점 비율 (예: 2.5 - 2 = 0.5)
	float LerpAlpha = FloatIndex - Index0;

	// 캐싱된 두 점의 트랜스폼 가져오기
	const FTransform& Transform0 = CachedSplineTransforms[Index0];
	const FTransform& Transform1 = CachedSplineTransforms[Index1];

	// 두 점 사이를 선형 보간하여 최종 트랜스폼 계산
	FTransform Result;
	Result.Blend(Transform0, Transform1, LerpAlpha);
    
	return Result;
}

FTransform AFactoryBelt::GetItemRenderTransform(float CycleAlpha) const
{
	float FinalAlpha = bIsBeltStop ? 1.0f : CycleAlpha;
	return GetSpineDistance(FinalAlpha);
}

void AFactoryBelt::CacheSplineTransforms()
{
	CachedSplineTransforms.Empty(SplineCacheSteps + 1);
	for (int32 i = 0; i <= SplineCacheSteps; ++i)
	{
		// 0.0 부터 1.0 까지의 진행도
		float Alpha = static_cast<float>(i) / SplineCacheSteps;
		float TargetDistance = Alpha * TotalSpineLength;
        
		FVector Loc = SplineComponent->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
		FRotator Rot = SplineComponent->GetRotationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
        
		CachedSplineTransforms.Add(FTransform(Rot, Loc));
	}
}

#pragma endregion

#pragma region interact logic

bool AFactoryBelt::TryGetInteractionOptions(const EPlacementMode CurrentMode,
	TArray<FInteractionOption>& OutOptions) const
{
	if (CurrentMode == EPlacementMode::Retrieve)
	{
		// 전체 수납
		FInteractionOption MassOption;
		MassOption.OptionID = TEXT("MassRetrieve");
		MassOption.DisplayText = FText::FromString(TEXT("컨베이어 벨트 라인 전체 수납"));
		OutOptions.Add(MassOption);
		
		// 단일 수납
		FInteractionOption SingleOption;
		SingleOption.OptionID = TEXT("SingleRetrieve");
		SingleOption.DisplayText = FText::FromString(TEXT("컨베이어 벨트 단일 수납"));
		OutOptions.Add(SingleOption);
		
		return true;
	}
	return false;
}

void AFactoryBelt::Interact(const AActor* Interactor, const EPlacementMode CurrentMode, int32 OptionIndex)
{
	if (CurrentMode == EPlacementMode::Retrieve)
	{
		if (OptionIndex == 0)
		{
			MassRetrieve();
		}
		else if (OptionIndex == 1)
		{
			Retrieve();
		}
	}
}

void AFactoryBelt::MassRetrieve()
{
	TSet<AFactoryBelt*> BeltLine = GetConnectedBeltLine();
	
	for (AFactoryBelt* Belt : BeltLine)
	{
		if (Belt) Belt->Retrieve();
	}
}

TSet<AFactoryBelt*> AFactoryBelt::GetConnectedBeltLine()
{
	TSet<AFactoryBelt*> VisitedBelts;
	TQueue<AFactoryBelt*> Queue;
	
	Queue.Enqueue(this);
	VisitedBelts.Add(this);
	
	while (!Queue.IsEmpty())
	{
		AFactoryBelt* CurrentBelt;
		Queue.Dequeue(CurrentBelt);
		
		// 정방향 탐색
		if (CurrentBelt->LogisticsOutputPortArr.IsValidIndex(0) && CurrentBelt->LogisticsOutputPortArr[0])
		{
			if (UFactoryInputPortComponent* ConnectedIn = CurrentBelt->LogisticsOutputPortArr[0]->GetConnectedInput())
			{
				if (AFactoryBelt* NextBelt = Cast<AFactoryBelt>(ConnectedIn->GetPortOwner()))
				{
					if (!VisitedBelts.Contains(NextBelt))
					{
						VisitedBelts.Add(NextBelt);
						Queue.Enqueue(NextBelt);
					}
				}
			}
		}
		
		// 역방향 탐색
		if (CurrentBelt->LogisticsInputPortArr.IsValidIndex(0) && CurrentBelt->LogisticsInputPortArr[0])
		{
			if (UFactoryOutputPortComponent* ConnectedOut = CurrentBelt->LogisticsInputPortArr[0]->GetConnectedOutput())
			{
				if (AFactoryBelt* PrevBelt = Cast<AFactoryBelt>(ConnectedOut->GetPortOwner()))
				{
					if (!VisitedBelts.Contains(PrevBelt))
					{
						VisitedBelts.Add(PrevBelt);
						Queue.Enqueue(PrevBelt);
					}
				}
			}
		}
	}
	
	return VisitedBelts;
}

#pragma endregion


