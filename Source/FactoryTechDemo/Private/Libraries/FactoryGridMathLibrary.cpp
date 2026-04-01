// Fill out your copyright notice in the Description page of Project Settings.


#include "Libraries/FactoryGridMathLibrary.h"

#include "Engine/OverlapResult.h"
#include "Logistics/Belts/FactoryBelt.h"
#include "Logistics/Ports/FactoryInputPortComponent.h"
#include "Logistics/Ports/FactoryOutputPortComponent.h"
#include "Logistics/Ports/FactoryPortComponentBase.h"

FVector UFactoryGridMathLibrary::GridToWorld(const FIntPoint& GridLocation, float GridLength, float Height)
{
	return FVector(
	(GridLocation.X * GridLength) + (GridLength * 0.5f),
	(GridLocation.Y * GridLength) + (GridLength * 0.5f),
	Height
	);
}

FIntPoint UFactoryGridMathLibrary::WorldToGrid(const FVector& WorldLocation, float GridLength)
{
	return FIntPoint(
		FMath::FloorToInt(WorldLocation.X / GridLength),
		FMath::FloorToInt(WorldLocation.Y / GridLength)
	);
}

FVector UFactoryGridMathLibrary::CalculateSnappedLocation(const FVector& RawLocation, const FIntPoint& GridSize, float GridLength)
{
	auto SnapValue = [](float Raw, int32 Size, float GridLength) -> float
	{
		float GridStart = FMath::FloorToFloat(Raw / GridLength) * GridLength;
		float Offset = (Size % 2 != 0) ? GridLength * 0.5f : 0.0f;
		return GridStart + Offset;
	};

	return FVector(SnapValue(RawLocation.X, GridSize.X, GridLength), SnapValue(RawLocation.Y, GridSize.Y, GridLength), RawLocation.Z);
}

TArray<AFactoryLogisticsObjectBase*> UFactoryGridMathLibrary::GetFacilitiesInGridBox(
	const UObject* WorldContextObject, const FIntPoint& StartGrid, const FIntPoint& EndGrid, float GridLength)
{
	TArray<AFactoryLogisticsObjectBase*> FoundFacilities;
	if (!WorldContextObject || !WorldContextObject->GetWorld()) return FoundFacilities;
	
	int32 MinX = FMath::Min(StartGrid.X, EndGrid.X);
	int32 MinY = FMath::Min(StartGrid.Y, EndGrid.Y);
	int32 MaxX = FMath::Max(StartGrid.X, EndGrid.X);
	int32 MaxY = FMath::Max(StartGrid.Y, EndGrid.Y);

	float CenterX = (MinX + MaxX + 1) * GridLength * 0.5f;
	float CenterY = (MinY + MaxY + 1) * GridLength * 0.5f;
	FVector CenterLoc(CenterX, CenterY, 50.f);
	
	float Margin = 2.0f; 
	float ExtentX = ((MaxX - MinX + 1) * GridLength * 0.5f) - Margin;
	float ExtentY = ((MaxY - MinY + 1) * GridLength * 0.5f) - Margin;
	FVector BoxExtent(ExtentX, ExtentY, 100.f);

	FCollisionShape BoxShape = FCollisionShape::MakeBox(BoxExtent);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel3);
    
	TArray<FOverlapResult> Overlaps;
	WorldContextObject->GetWorld()->OverlapMultiByObjectType(Overlaps, CenterLoc, FQuat::Identity, ObjectParams, BoxShape);

	for (const FOverlapResult& Result : Overlaps)
	{
		if (AFactoryLogisticsObjectBase* HitObj = Cast<AFactoryLogisticsObjectBase>(Result.GetActor()))
		{
			FoundFacilities.Add(HitObj);
		}
	}
	return FoundFacilities;
}

AFactoryLogisticsObjectBase* UFactoryGridMathLibrary::GetFacilityAtGrid(const UObject* WorldContextObject, const FVector& GridLocation, float GridLength)
{
	FIntPoint TargetGrid = WorldToGrid(GridLocation, GridLength);
	TArray<AFactoryLogisticsObjectBase*> Facilities = GetFacilitiesInGridBox(WorldContextObject, TargetGrid, TargetGrid, GridLength);
	return Facilities.Num() > 0 ? Facilities[0] : nullptr;
}

bool UFactoryGridMathLibrary::TryGetSmartSnapPortGrid(const FVector& PointingLocation, AActor* HitActor, bool bIsOutput,
	FIntPoint& OutGrid, UFactoryPortComponentBase*& OutPort, float GridLength)
{
	AFactoryLogisticsObjectBase* Facility = Cast<AFactoryLogisticsObjectBase>(HitActor);
	if (!Facility) return false;
	if (Facility->IsA<AFactoryBelt>())
		return false;
	
	OutPort = nullptr;
	
	TArray<UFactoryPortComponentBase*> PortsToSearch;
	if (bIsOutput)
	{
		for (auto* Port : Facility->GetOutputPorts()) { PortsToSearch.Add(Port); }
	}
	else
	{
		for (auto* Port : Facility->GetInputPorts()) { PortsToSearch.Add(Port); }
	}
	
	float MinDistSqr = FLT_MAX;
	for (auto* Port : PortsToSearch)
	{
		float DistSqr = FVector::DistSquared(Port->GetComponentLocation(), PointingLocation);
		if (DistSqr < MinDistSqr)
		{
			MinDistSqr = DistSqr;
			OutPort = Port;
		}
	}
	
	if (OutPort)
	{
		FVector OutportDir = OutPort->GetForwardVector();
		int32 Sign = bIsOutput ? 1 : -1;
		FVector PortForwardVector = OutPort->GetComponentLocation() + (OutportDir * GridLength * Sign);
		FVector SnappedPortForwardVector = CalculateSnappedLocation(
			PortForwardVector, FIntPoint(1,1), GridLength);	// 결과값도 안전하게 스냅
		OutGrid = WorldToGrid(SnappedPortForwardVector, GridLength);
		return true;
	}
	return false;
}

bool UFactoryGridMathLibrary::TryFindNearPortDirection(const UObject* WorldContextObject, const FVector& SearchCenter, bool bFindInputPort,
	FVector& OutDir, float GridLength)
{
	if (!WorldContextObject || !WorldContextObject->GetWorld()) return false;
	
	int XArr[4] = {1, -1, 0, 0};
	int YArr[4] = {0, 0, 1, -1};
	FVector StartLocation = SearchCenter + FVector(0, 0, 5.f); // 지면 간섭 방지
	float TraceLength = GridLength; 
	FHitResult PortHit;
	
	for (int i = 0; i < 4; i++)
	{
		FVector SearchEnd = StartLocation + FVector(XArr[i] * TraceLength, YArr[i] * TraceLength, 0.f);
        
		if (WorldContextObject->GetWorld()->LineTraceSingleByChannel(PortHit, StartLocation, SearchEnd, ECC_GameTraceChannel2))	// Port
		{
			if (bFindInputPort)
			{
				if (UFactoryInputPortComponent* InputPort = Cast<UFactoryInputPortComponent>(PortHit.GetComponent()))
				{
					OutDir = InputPort->GetForwardVector();
					return true;
				}
			}
			else
			{
				if (UFactoryOutputPortComponent* OutputPort = Cast<UFactoryOutputPortComponent>(PortHit.GetComponent()))
				{
					OutDir = OutputPort->GetForwardVector();
					return true;
				}
			}
		}
	}
	return false;
}

EBeltType UFactoryGridMathLibrary::DetermineBeltType(const FVector& StartDir, const FVector& EndDir)
{
	if (StartDir.Equals(EndDir, 0.01f))
	{
		return EBeltType::Straight;
	}
	
	float CrossZ = FVector::CrossProduct(StartDir, EndDir).Z;
	if (CrossZ < 0.f) return EBeltType::LeftTurn;
	if (CrossZ > 0.f) return EBeltType::RightTurn;
	
	return EBeltType::Straight;
}




