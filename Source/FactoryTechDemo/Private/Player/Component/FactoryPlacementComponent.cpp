// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/FactoryPlacementComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Logistics/FactoryLogisticsObjectBase.h"
#include "Placement/FactoryObjectData.h"
#include "Placement/FactoryPlaceObjectBase.h"
#include "Placement/FactoryPlacePreview.h"
#include "Player/FactoryPlayerController.h"
#include "Settings/FactoryBuildingSettings.h"


UFactoryPlacementComponent::UFactoryPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFactoryPlacementComponent::BeginPlay()
{
	Super::BeginPlay();
    
    if (const UFactoryBuildingSettings* Settings = GetDefault<UFactoryBuildingSettings>())
    {
        GridLength = Settings->GetGridLength();
    }
	
    CachedPlayerController = Cast<AFactoryPlayerController>(GetOwner());
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName("PlaceObjectPivotActor");
	PlaceObjectPivotActor = GetWorld()->SpawnActor<AActor>(SpawnParams);
	USceneComponent* PivotRoot = NewObject<USceneComponent>(PlaceObjectPivotActor, TEXT("PivotRoot"));
	PlaceObjectPivotActor->SetRootComponent(PivotRoot);
	PivotRoot->RegisterComponent();
	PlaceObjectPivotActor->SetHidden(true);
}

void UFactoryPlacementComponent::UpdatePreviewState()
{
	if (CurrentPlacementMode == EPlacementMode::None || !PlaceObjectPivotActor) return;
	
	// 마우스 위치로 피벗 스냅
	FVector NewLocation = GetPlacementObjectLocation();
	PlaceObjectPivotActor->SetActorLocation(NewLocation);
	
	// 유효성(충돌) 검사
	bool bGlobalValid = true;
	for (AFactoryPlacePreview* Preview : ActivePreviews)
	{
		if (!Preview->UpdateOverlapValidity())
		{
			bGlobalValid = false;
		}
	}
	
	// 유효성 검사 결과 적용
	for (AFactoryPlacePreview* Preview : ActivePreviews)
	{
		Preview->SetVisualValidity(bGlobalValid);
	}
}

#pragma region 프리뷰 제어

void UFactoryPlacementComponent::SetFirstPlacePreview(UFactoryObjectData* Data)
{
	if (!Data) return;
	ClearAllPreviews();
	
	AFactoryPlacePreview* Preview = GetWorld()->SpawnActor<AFactoryPlacePreview>();
	Preview->InitPreview(Data);
	ActivePreviews.Add(Preview);
	
	CurrentPlacementMode = EPlacementMode::FirstPlace;
	StartObjectPlaceMode();
}

void UFactoryPlacementComponent::SetMoveObjectToPreviews()
{
	if (SelectedLogisticsObjectBases.Num() == 0) return;
	ClearAllPreviews();
	
	for (auto& LogisticsObjectBase : SelectedLogisticsObjectBases)
	{
		AFactoryPlacePreview* Preview = GetWorld()->SpawnActor<AFactoryPlacePreview>();
		Preview->InitPreview(LogisticsObjectBase->GetObjectData());
		Preview->SetActorLocationAndRotation(
			LogisticsObjectBase->GetActorLocation(), LogisticsObjectBase->GetActorRotation());
		ActivePreviews.Add(Preview);
	}
	
	CurrentPlacementMode = EPlacementMode::Move;
	StartObjectPlaceMode();
}

void UFactoryPlacementComponent::ClearAllPreviews()
{
	for (auto Preview : ActivePreviews)
	{
		if (Preview) Preview->Destroy();
	}
	ActivePreviews.Empty();
	
	if (PlaceObjectPivotActor)
	{
		PlaceObjectPivotActor->SetHidden(true);
	}
	
	CurrentPlacementMode = EPlacementMode::None;
}

void UFactoryPlacementComponent::StartObjectPlaceMode()
{
	if (ActivePreviews.Num() <= 0) return;
	if (PlaceObjectPivotActor)
	{
		PlaceObjectPivotActor->SetHidden(false);
		CalculatePlacementPivotCenterAndGridSize();
		
		for (auto& Preview : ActivePreviews)
		{
			Preview->AttachToActor(PlaceObjectPivotActor, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
}

void UFactoryPlacementComponent::RotatePlacementPreview() const
{
	if (!PlaceObjectPivotActor) return;
	// 피벗 자체를 돌리면 자식 프리뷰들이 함께 회전함
	float NextYaw = FMath::GridSnap(PlaceObjectPivotActor->GetActorRotation().Yaw + 90.f, 90.f);
	PlaceObjectPivotActor->SetActorRotation(FRotator(0.f, FRotator::NormalizeAxis(NextYaw), 0.f));
}

void UFactoryPlacementComponent::PlaceObject()
{
	bool bGlobalValid = true;
	for (auto Preview : ActivePreviews) { if (!Preview->GetPlacementValid()) bGlobalValid = false; }

	if (bGlobalValid && ActivePreviews.Num() > 0)
	{
		for (auto Preview : ActivePreviews)
		{
			FActorSpawnParameters Params; 
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			GetWorld()->SpawnActor<AFactoryPlaceObjectBase>(
				Preview->GetObjectData()->PlaceObjectBP, 
				Preview->GetActorLocation(), 
				Preview->GetActorRotation(), 
				Params);
		}
		ClearAllPreviews();
	}
	
	if (CurrentPlacementMode == EPlacementMode::Move)
	{
		for (auto& LogisticsObjectBase : SelectedLogisticsObjectBases)
		{
			LogisticsObjectBase->Destroy();
		}
	}
}

void UFactoryPlacementComponent::CancelPlaceObject()
{
	ClearAllPreviews();
}

void UFactoryPlacementComponent::CalculatePlacementPivotCenterAndGridSize()
{
	if (CurrentPlacementMode == EPlacementMode::FirstPlace && ActivePreviews.IsValidIndex(0))
	{
		if (const UFactoryObjectData* Data = ActivePreviews[0]->GetObjectData())
		{
			PlaceObjectPivotGridSize = Data->GridSize;
			PlaceObjectPivotActor->SetActorLocation(ActivePreviews[0]->GetActorLocation());
		}
	}
	else if (CurrentPlacementMode == EPlacementMode::Move && SelectedLogisticsObjectBases.Num() > 0)
	{
		// 모든 프리뷰 위치 포함 최소/최대 좌표 계산
		FVector Min(FLT_MAX), Max(-FLT_MAX);
		for (const auto& Preview : ActivePreviews)
		{
			FVector Loc = Preview->GetActorLocation();
			Min = Min.ComponentMin(Loc);
			Max = Max.ComponentMax(Loc);
		}

		// 중앙 지점으로 피벗 이동
		FVector Center = (Min + Max) * 0.5f;
		PlaceObjectPivotActor->SetActorLocation(Center);

		// 전체 그리드 크기 계산
		int32 SizeX = FMath::RoundToInt((Max.X - Min.X) / GridLength) + 1;
		int32 SizeY = FMath::RoundToInt((Max.Y - Min.Y) / GridLength) + 1;
		PlaceObjectPivotGridSize = FIntPoint(SizeX, SizeY);
	}
}

FVector UFactoryPlacementComponent::GetPlacementObjectLocation() const
{
	if (ActivePreviews.Num() <= 0) return FVector::ZeroVector;

	FHitResult HitResult;
	bool bHit = false;
	
	if (AFactoryPlayerController* Controller = CachedPlayerController.Get())
	{
		if (Controller->GetCurrentViewMode() == EFactoryViewModeType::NormalView)
		{
			FVector CameraLocation; FRotator CameraRotation;
			Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
			FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 1500.f); // MaxBuildTraceDistance
        
			FCollisionQueryParams Params; 
			Params.AddIgnoredActor(Controller->GetPawn());
			bHit = GetWorld()->LineTraceSingleByChannel(
				HitResult, CameraLocation, TraceEnd, ECC_GameTraceChannel1, Params);
        
			if (!bHit)
			{
				bHit = GetWorld()->LineTraceSingleByChannel(
					HitResult, TraceEnd, TraceEnd + (FVector::DownVector * 1500.f), ECC_GameTraceChannel1, Params);
			}
		}
		else
		{
			bHit = Controller->GetHitResultUnderCursor(ECC_GameTraceChannel1, false, HitResult);
		}
	}
    
	if (!bHit) return FVector::ZeroVector;
	return CalculateSnappedLocation(HitResult.Location, PlaceObjectPivotGridSize);
}

FVector UFactoryPlacementComponent::CalculateSnappedLocation(FVector RawLocation, FIntPoint GridSize) const
{
	auto SnapValue = [this](float Raw, int32 Size) -> float
	{
		float GridStart = FMath::FloorToFloat(Raw / GridLength) * GridLength;
		float Offset = (Size % 2 != 0) ? GridLength * 0.5f : 0.0f;
		return GridStart + Offset;
	};

	return FVector(SnapValue(RawLocation.X, GridSize.X), SnapValue(RawLocation.Y, GridSize.Y), RawLocation.Z);
}

#pragma endregion 

#pragma region 객체 선택

void UFactoryPlacementComponent::SelectObject(AFactoryLogisticsObjectBase* TargetObject)
{
	SelectedLogisticsObjectBases.Add(TargetObject);
	//TODO : 선택된 객체들에 대한 시각적 피드백(예: 하이라이트) 구현
}

void UFactoryPlacementComponent::DeselectObject(AFactoryLogisticsObjectBase* TargetObject)
{
	SelectedLogisticsObjectBases.Remove(TargetObject);
	//TODO : 선택 해제된 객체에 대한 시각적 피드백 제거 구현
}

void UFactoryPlacementComponent::ClearObject()
{
	for (auto Object : SelectedLogisticsObjectBases)
	{
		DeselectObject(Object);
	}
}

#pragma endregion

// TArray<FBeltPlacementData> UFactoryPlacementComponent::CalculateBeltPath(FIntPoint StartPoint, FIntPoint EndPoint) const
// {
// 		CurrentPlacementMode = EPlacementMode::BeltPlace;
// 	
// }
