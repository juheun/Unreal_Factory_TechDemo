// Fill out your copyright notice in the Description page of Project Settings.


#include "Logistics/FactoryBelt.h"

#include "Components/SplineComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Logistics/FactoryInputPortComponent.h"
#include "Items/FactoryItemData.h"
#include "Settings/FactoryBuildingSettings.h"
#include "Subsystems/FactoryCycleSubsystem.h"


AFactoryBelt::AFactoryBelt()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFactoryBelt::PlanCycle()
{
	Super::PlanCycle();
	
	// 벨트는 무조건 Output이 하나라는 가정하에 index 0사용
	if (!CurrentItem.ItemData || !LogisticsOutputPortArr[0]) return;
	
	UFactoryInputPortComponent* TargetPort = LogisticsOutputPortArr[0]->GetConnectedInput();
	if (!TargetPort) return;
	
	if (TargetPort->GetPortOwner()->CanPushItemFromBeforeObject(TargetPort))
	{
		TargetPort->PendingItem = CurrentItem;	// 상대방 Input에 아이템 밀어넣기
		CurrentItem = FFactoryItemInstance();
	}
}

void AFactoryBelt::ExecuteCycle()
{
	Super::ExecuteCycle();
	
	// InputPort에 Pending된 아이템이 있으면 가져옴
	if (!LogisticsInputPortArr[0]) return;
	if (LogisticsInputPortArr[0]->PendingItem.IsValid())
	{
		PullItemFromInputPorts(LogisticsInputPortArr[0]->PendingItem);
		LogisticsInputPortArr[0]->PendingItem = FFactoryItemInstance();
	}
}

void AFactoryBelt::UpdateView()
{
	Super::UpdateView();
	
	SetActorTickEnabled(CurrentItem.IsValid());
	if (CurrentItem.IsValid() && CurrentItem.VisualActor)
	{
		FVector StartLocation = SplineComponent->GetLocationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
		CurrentItem.VisualActor->SetActorLocation(StartLocation);
		CurrentItem.VisualActor->SetActorHiddenInGame(false);
	}
}

void AFactoryBelt::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (CurrentItem.IsValid() && CurrentItem.VisualActor)
	{
		float Alpha = GetWorld()->GetSubsystem<UFactoryCycleSubsystem>()->GetCycleAlpha();
		
		float TotalLength = SplineComponent->GetSplineLength();
		float TargetDistance = Alpha * TotalLength;
		
		FVector NewLoc = SplineComponent->GetLocationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
		FRotator NewRot = SplineComponent->GetRotationAtDistanceAlongSpline(TargetDistance, ESplineCoordinateSpace::World);
		
		CurrentItem.VisualActor->SetActorRotation(NewRot);
	}
}

bool AFactoryBelt::CanPushItemFromBeforeObject(const UFactoryInputPortComponent* RequestPort) const
{
	//return Super::CanPushItemFromBeforeObject(RequestPort);
	if (!LogisticsInputPortArr[0]) return false;
	const UFactoryItemData* PendingItem = LogisticsInputPortArr[0]->PendingItem.ItemData;
	return !PendingItem && !CurrentItem.ItemData;
}

void AFactoryBelt::PullItemFromInputPorts(FFactoryItemInstance& Item)
{
	Super::PullItemFromInputPorts(Item);
	
	CurrentItem = Item;
}

void AFactoryBelt::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	UpdateSplinePath(BeltType);
}

void AFactoryBelt::UpdateSplinePath(EBeltType Type)
{
	if (!SplineComponent) return;
	SplineComponent->ClearSplinePoints(true);
	
	float GridLength = GetDefault<UFactoryBuildingSettings>()->GetGridLength();
	float HalfGridLength = GridLength * 0.5f;
	float CurveStrength = HalfGridLength; // 곡률의 부드러움 결정 (보통 변의 길이의 절반 정도)
	
	// 시작점 (Input Port 위치)
	FVector StartPos = FVector(-HalfGridLength, 0.0f, 0.0f); // 벨트 뒤쪽 끝
	FVector StartTangent = FVector(CurveStrength * 2.0f, 0.0f, 0.0f); // 앞을 향하는 힘

	// 끝점 (Output Port 위치)
	FVector EndPos;
	FVector EndTangent;
	
	switch (Type)
	{
	case EBeltType::Straight:
		EndPos = FVector(HalfGridLength, 0.0f, 0.0f);
		EndTangent = FVector(CurveStrength * 2.0f, 0.0f, 0.0f);
		break;

	case EBeltType::LeftTurn:
		EndPos = FVector(0.0f, -HalfGridLength, 0.0f); // 왼쪽으로 90도
		EndTangent = FVector(0.0f, -CurveStrength * 2.0f, 0.0f);
		break;

	case EBeltType::RightTurn:
		EndPos = FVector(0.0f, HalfGridLength, 0.0f); // 오른쪽으로 90도
		EndTangent = FVector(0.0f, CurveStrength * 2.0f, 0.0f);
		break;
	}

	// 스플라인 포인트 추가 (로컬 좌표계 기준)
	SplineComponent->AddSplinePoint(StartPos, ESplineCoordinateSpace::Local, true);
	SplineComponent->SetTangentAtSplinePoint(0, StartTangent, ESplineCoordinateSpace::Local, true);

	SplineComponent->AddSplinePoint(EndPos, ESplineCoordinateSpace::Local, true);
	SplineComponent->SetTangentAtSplinePoint(1, EndTangent, ESplineCoordinateSpace::Local, true);

	// 베지어 곡선으로 부드럽게 만들기 위해 포인트 타입 설정
	SplineComponent->SetSplinePointType(0, ESplinePointType::Curve, true);
	SplineComponent->SetSplinePointType(1, ESplinePointType::Curve, true);
}


