// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryBelt.h"

#include "Components/SplineComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryLogisticsTypes.h"
#include "Items/FactoryItemData.h"
#include "Items/FactoryItemVisual.h"
#include "Settings/FactoryDeveloperSettings.h"
#include "Subsystems/FactoryCycleSubsystem.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "Subsystems/FactoryWarehouseSubsystem.h"


AFactoryBelt::AFactoryBelt()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	SplineComponent = CreateDefaultSubobject<USplineComponent>("SplineComponent");
	
	SplineComponent->SetupAttachment(RootComponent);
}

void AFactoryBelt::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	UFactoryWarehouseSubsystem* WarehouseSubsystem = GetWorld()->GetSubsystem<UFactoryWarehouseSubsystem>();
	if (!WarehouseSubsystem) return;
	
	UFactoryPoolSubsystem* PoolSubsystem = GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
	if (!PoolSubsystem) return;
	
	if (CurrentItem.IsValid())
	{
		WarehouseSubsystem->StoreItem(
			const_cast<UFactoryItemData*>(CurrentItem.ItemData.Get()), 1);
		if (CurrentItem.VisualActor.Get())
		{
			PoolSubsystem->ReturnItemToPool(CurrentItem.VisualActor.Get()); 
		}
	}
	
	//Pending된 아이템도 제거
	if (LogisticsInputPortArr.IsValidIndex(0) && LogisticsInputPortArr[0])
	{
		FFactoryItemInstance& PendingItem = LogisticsInputPortArr[0]->PendingItem;
		if (PendingItem.IsValid())
		{
			WarehouseSubsystem->StoreItem(
				const_cast<UFactoryItemData*>(PendingItem.ItemData.Get()), 1);
			
			if (PendingItem.VisualActor.IsValid())
			{
				PoolSubsystem->ReturnItemToPool(PendingItem.VisualActor.Get()); 
			}
		}
	}
}

void AFactoryBelt::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	SetBeltType(BeltType);
}

void AFactoryBelt::PlanCycle()
{
	// 벨트는 무조건 Output이 하나라는 가정하에 index 0사용
	if (!CurrentItem.ItemData || !LogisticsOutputPortArr.IsValidIndex(0)) return;
	
	bIsBeltStop = true;
	
	UFactoryInputPortComponent* TargetPort = LogisticsOutputPortArr[0]->GetConnectedInput();
	if (!TargetPort) return;
	
	if (TargetPort->GetPortOwner()->CanPushItemFromBeforeObject(TargetPort))
	{
		TargetPort->PendingItem = CurrentItem;	// 상대방 Input에 아이템 밀어넣기
		CurrentItem = FFactoryItemInstance();
		bIsBeltStop = false;
	}
}

void AFactoryBelt::ExecuteCycle()
{
	// InputPort에 Pending된 아이템이 있으면 가져옴
	if (!LogisticsInputPortArr.IsValidIndex(0) || !LogisticsInputPortArr[0]) return;
	if (LogisticsInputPortArr[0]->PendingItem.IsValid())
	{
		PullItemFromInputPorts(LogisticsInputPortArr[0]->PendingItem);
		LogisticsInputPortArr[0]->PendingItem = FFactoryItemInstance();
		bIsBeltStop = false;
	}
}

void AFactoryBelt::UpdateView()
{
	AFactoryItemVisual* ItemVisual = CurrentItem.VisualActor.Get();
	if (ItemVisual)
	{
		float SpineAlpha = bIsBeltStop ? 1.f : 0.f;
		SetSpineDistance(SpineAlpha);
		ItemVisual->SetActorHiddenInGame(false);
	}
	SetActorTickEnabled(!bIsBeltStop && ItemVisual);
}

void AFactoryBelt::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	AFactoryItemVisual* ItemVisual = CurrentItem.VisualActor.Get();
	if (ItemVisual)
	{
		UFactoryCycleSubsystem* CycleSubsystem = GetWorld()->GetSubsystem<UFactoryCycleSubsystem>();
		if (!CycleSubsystem) return;
		float Alpha = CycleSubsystem->GetCycleAlpha();
		SetSpineDistance(Alpha);
	}
}

bool AFactoryBelt::CanPushItemFromBeforeObject(const UFactoryInputPortComponent* RequestPort) const
{
	if (!LogisticsInputPortArr.IsValidIndex(0)) return false;
	const UFactoryItemData* PendingItem = LogisticsInputPortArr[0]->PendingItem.ItemData;
	return !PendingItem && !CurrentItem.ItemData;
}

void AFactoryBelt::SetBeltType(EBeltType Type)
{
	BeltType = Type;
	UpdateSplinePath(Type);
	UpdateBeltVisual(Type);
}

bool AFactoryBelt::PullItemFromInputPorts(FFactoryItemInstance& Item)
{
	CurrentItem = Item;
	return true;
}

void AFactoryBelt::UpdateSplinePath(EBeltType Type)
{
	if (!SplineComponent) return;
	SplineComponent->ClearSplinePoints(true);
	
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
		FVector OutPutPort = EndPos;
		OutPutPort.Z = BeltHeight * 0.5f;
		LogisticsOutputPortArr[0]->SetRelativeLocation(OutPutPort);
		float TargetYaw = 0.f;
		if (Type == EBeltType::LeftTurn) TargetYaw = -90.f;
		else if (Type == EBeltType::RightTurn) TargetYaw = 90.f;
		
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

void AFactoryBelt::SetSpineDistance(float Alpha)
{
	float TargetDistance = Alpha * TotalSpineLength;
		
	FVector NewLoc = SplineComponent->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
	FRotator NewRot = SplineComponent->GetRotationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
	
	if (AFactoryItemVisual* ItemVisual = CurrentItem.VisualActor.Get())
	{
		ItemVisual->SetActorLocationAndRotation(NewLoc, NewRot);
	}
}


