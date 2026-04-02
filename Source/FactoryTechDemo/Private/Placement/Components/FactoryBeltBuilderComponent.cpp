// Fill out your copyright notice in the Description page of Project Settings.


#include "Placement/Components/FactoryBeltBuilderComponent.h"

#include "Libraries/FactoryGridMathLibrary.h"
#include "Logistics/FactoryLogisticsTypes.h"
#include "Logistics/Belts/FactoryBelt.h"
#include "Logistics/Belts/FactoryBeltBridge.h"
#include "Logistics/Ports/FactoryPortComponentBase.h"


UFactoryBeltBuilderComponent::UFactoryBeltBuilderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFactoryBeltBuilderComponent::ResetBuilderState()
{
	bIsWaitingDetermineBeltEnd = false;
	BeltStartPoint = FIntPoint::ZeroValue;
	BeltStartDir = FVector::ZeroVector;
}

bool UFactoryBeltBuilderComponent::GetPreviewPathData(
	const FVector& PointingLocation, float GridLength, bool bAlternativeRoute, TArray<FBeltPlacementData>& OutPathData) const
{
	if (!bIsWaitingDetermineBeltEnd) return false;	// 첫 시작지점이 정해져야 도는 함수임
	
	OutPathData.Empty();
	
	FIntPoint ResultPoint = UFactoryGridMathLibrary::WorldToGrid(PointingLocation, GridLength);
	UFactoryPortComponentBase* TargetPort = nullptr; 
	
	// 목적지 포트 스냅 검사. 마우스가 위치한곳에 설비가 있으면 해당 설비의 InputPot앞으로 자동 스냅
	if (AFactoryLogisticsObjectBase* HitFacility = UFactoryGridMathLibrary::GetFacilityAtGrid(this, PointingLocation, GridLength))
	{
		FIntPoint SnapGrid;
		if (UFactoryGridMathLibrary::TryGetSmartSnapPortGrid(
			PointingLocation, HitFacility, false, SnapGrid, TargetPort, GridLength))
		{
			ResultPoint = SnapGrid;
		}
	}
	
	OutPathData = CalculateBeltPath(BeltStartPoint, ResultPoint, BeltStartDir, GridLength, bAlternativeRoute, TargetPort);
	return OutPathData.Num() > 0;
}

FVector UFactoryBeltBuilderComponent::GetSnappedStartLocation(const FVector& PointingLocation, float GridLength) const
{
	FVector FinalLocation = PointingLocation;
             
	// 마우스 위치에 설비가 있다면 Output 포트에 자동으로 스냅
	if (AFactoryLogisticsObjectBase* HitFacility = UFactoryGridMathLibrary::GetFacilityAtGrid(this, PointingLocation, GridLength))
	{
		FIntPoint SnapGrid;
		UFactoryPortComponentBase* TargetPort = nullptr;
		if (UFactoryGridMathLibrary::TryGetSmartSnapPortGrid(PointingLocation, HitFacility, true, SnapGrid, TargetPort, GridLength))
		{
			FinalLocation = UFactoryGridMathLibrary::GridToWorld(SnapGrid, GridLength);
		}
	}
    
	return FinalLocation;
}

bool UFactoryBeltBuilderComponent::ProcessClick(const FVector& PointingLocation, float GridLength)
{
	if (!bIsWaitingDetermineBeltEnd)	// 시작점이 안 찍힌 상태면
	{
		// 찍힌 위치가 벨트 시작점으로 유효한 위치인지 검사
		if (TryGetBeltStartData(PointingLocation, GridLength, BeltStartPoint, BeltStartDir))
		{
			bIsWaitingDetermineBeltEnd = true;
			return false;	// 종료점 미정. 배치 시작X
		}
	}
	else
	{
		return true;	// 종료점 확정. 배치 시작
	}
	return false;
}

TArray<FBeltPlacementData> UFactoryBeltBuilderComponent::CalculateBeltPath(const FIntPoint& StartPoint,
	const FIntPoint& EndPoint, const FVector& StartPointDir, float GridLength, bool bAlternativeRoute,
	UFactoryPortComponentBase* TargetPort) const
{
	TArray<FIntPoint> Points;
	Points.Add(StartPoint);

	// 목적지까지의 경로의 포인트 배열 만들기
	if (StartPoint != EndPoint)
	{
		// 지정한 축으로 목표 지점까지 한 칸씩 배열에 추가하며 전진하는 람다 함수
		auto MoveToX = [&](int32 TargetX) {
			int32 Step = FMath::Sign(TargetX - Points.Last().X);
			while (Points.Last().X != TargetX) { Points.Add(FIntPoint(Points.Last().X + Step, Points.Last().Y)); }
		};
		auto MoveToY = [&](int32 TargetY) {
			int32 Step = FMath::Sign(TargetY - Points.Last().Y);
			while (Points.Last().Y != TargetY) { Points.Add(FIntPoint(Points.Last().X, Points.Last().Y + Step)); }
		};

		FIntPoint StartDirInt(FMath::RoundToInt(StartPointDir.X), FMath::RoundToInt(StartPointDir.Y));
		bool bIsXDir = (StartDirInt.X != 0);
		// 목적지가 바라보는 방향의 바로 뒤에 있는지 검사
		bool bTargetIsBehind = (bIsXDir && FMath::Sign(EndPoint.X - StartPoint.X) == -StartDirInt.X) || (!bIsXDir && FMath::Sign(EndPoint.Y - StartPoint.Y) == -StartDirInt.Y);

		if (!bTargetIsBehind) {
			// 목적지가 앞이나 옆에 있으면, 내가 바라보는 방향을 최우선으로 먼저 이동
			if (bIsXDir)
			{
				MoveToX(EndPoint.X); 
				MoveToY(EndPoint.Y);
			}
			else
			{
				MoveToY(EndPoint.Y); 
				MoveToX(EndPoint.X);
			}
		}
		else {
			// 목적지가 등 뒤에 있으면, 바로 뒤로 돌 수 없으므로 좌/우 측면으로 한 칸 빠진 뒤 이동
			int32 AltSign = bAlternativeRoute ? -1 : 1;
			if (bIsXDir)
			{
				if (StartPoint.Y == EndPoint.Y)
				{
					MoveToY(StartPoint.Y + AltSign); 
					MoveToX(EndPoint.X); 
					MoveToY(EndPoint.Y);
				}
				else
				{
					MoveToY(EndPoint.Y);
					MoveToX(EndPoint.X);
				}
			}
			else  // 바라보는 방향이 Y축일 때
			{
				if (StartPoint.X == EndPoint.X)
				{
					MoveToX(StartPoint.X + AltSign); 
					MoveToY(EndPoint.Y); 
					MoveToX(EndPoint.X);
				}
				else
				{
					MoveToX(EndPoint.X); 
					MoveToY(EndPoint.Y);
				}
			}
		}
	}
	
	// 계산된 그리드 좌표들을 기반으로 실제 설치될 벨트의 종류와 회전값을 결정
	TArray<FBeltPlacementData> OutBeltPath;
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		FVector CurrentLoc = UFactoryGridMathLibrary::GridToWorld(Points[i], GridLength);
		AFactoryLogisticsObjectBase* HitFacility = UFactoryGridMathLibrary::GetFacilityAtGrid(this, CurrentLoc, GridLength);
		
		// 만약 이미 브릿지가 위치해있다면 생략
		if (HitFacility && HitFacility->IsA<AFactoryBeltBridge>()) continue;

		FBeltPlacementData Data;
		Data.GridPoint = Points[i];
		// 이전 벨트에서 현재 벨트로의 방향
		FVector InDir = (i == 0) ? StartPointDir : FVector(Points[i] - Points[i - 1], 0.f).GetSafeNormal();
		FVector EndDir = InDir;
		bool bIsBridge = false;

		if (i < Points.Num() - 1)
		{
			EndDir = FVector(Points[i + 1] - Points[i], 0.f).GetSafeNormal();		// 중간 벨트는 무조건 다음 벨트를 향해 방향이 결정됨
			if (i > 0 && HitFacility)	
			{
				if (AFactoryBelt* HitBelt = Cast<AFactoryBelt>(HitFacility)) // 만약 벨트가 직교한다면 브릿지 생성 플래그 켜짐
				{
					if (HitBelt->GetBeltType() == EBeltType::Straight && FMath::Abs(FVector::DotProduct(HitBelt->GetActorForwardVector(), InDir)) < 0.1f)
						bIsBridge = true;
				}
			}
		}
		else 
		{	
			// 마지막 벨트의 경우
			if (TargetPort) EndDir = TargetPort->GetForwardVector(); // 타겟 포트가 정해져있다면 그 포트로 강제 연결
			else if (HitFacility)	// 설비가 지정됐지만 포트 지정은 없는 경우 = 벨트
			{
				if (AFactoryBelt* HitBelt = Cast<AFactoryBelt>(HitFacility)) EndDir = HitBelt->GetBeltExitDirection();
			}
			else
			{
				// 마지막 지점에서 4면으로 연결할 Port 탐색
				FVector OutDir = FVector::ZeroVector;
				UFactoryGridMathLibrary::TryFindNearPortDirection(this, CurrentLoc, true, OutDir, GridLength);
				EndDir = OutDir;
			}
		}

		Data.Type = UFactoryGridMathLibrary::DetermineBeltType(InDir, EndDir);
		Data.Rotation = InDir.Rotation();
		Data.bIsBridge = bIsBridge;
		OutBeltPath.Add(Data);
	}
	return OutBeltPath;
}

bool UFactoryBeltBuilderComponent::TryGetBeltStartData(const FVector& PointingLocation, float GridLength,
	FIntPoint& OutStartGrid, FVector& OutStartDir) const
{
	OutStartGrid = UFactoryGridMathLibrary::WorldToGrid(PointingLocation, GridLength);
	if (AActor* HitActor = UFactoryGridMathLibrary::GetFacilityAtGrid(this, PointingLocation, GridLength))
	{
		// 만약 시작위치에 벨트가 있다면 해당 벨트부터 연장
		if (AFactoryBelt* HitBelt = Cast<AFactoryBelt>(HitActor))
		{
			OutStartDir = HitBelt->GetActorForwardVector();
			return true;
		}
		
		FIntPoint SnapGrid;
		UFactoryPortComponentBase* Port = nullptr;
		
		// 벨트가 아니라면 해당 설비의 OutputPort중 가장 가까운 위치부터 벨트 연결
		if (UFactoryGridMathLibrary::TryGetSmartSnapPortGrid(PointingLocation, HitActor, true, SnapGrid, Port, GridLength))
		{
			OutStartGrid = SnapGrid;
			OutStartDir = Port->GetForwardVector();
			return true;
		}
	}
	// 모든 경우가 아닌 경우 4면 포트 찾기 검사 수행
	return UFactoryGridMathLibrary::TryFindNearPortDirection(this, PointingLocation, false, OutStartDir, GridLength);
}






