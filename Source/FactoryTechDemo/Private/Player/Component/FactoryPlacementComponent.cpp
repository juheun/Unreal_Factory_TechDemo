// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/FactoryPlacementComponent.h"

#include "Logistics/FactoryBelt.h"
#include "Logistics/FactoryLogisticsObjectBase.h"
#include "Logistics/FactoryLogisticsTypes.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Placement/FactoryBeltPreview.h"
#include "Placement/FactoryObjectData.h"
#include "Placement/FactoryPlaceObjectBase.h"
#include "Placement/FactoryPlacePreview.h"
#include "Player/FactoryPlayerController.h"
#include "Settings/FactoryDeveloperSettings.h"
#include "Subsystems/FactoryPoolSubsystem.h"


UFactoryPlacementComponent::UFactoryPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFactoryPlacementComponent::BeginPlay()
{
	Super::BeginPlay();
    
    if (const UFactoryDeveloperSettings* Settings = GetDefault<UFactoryDeveloperSettings>())
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
	FVector NewLocation;
	if (TryGetPointingGridLocation(NewLocation))
	{
		if (CurrentPlacementMode == EPlacementMode::BeltPlace && bIsWaitingDetermineBeltEnd)
		{
			FIntPoint EndPoint = WorldToGrid(NewLocation);
			BeltPlacePreviewUpdate(CalculateBeltPath(BeltStartPoint, EndPoint, BeltStartDir));
		}
		else
		{
			PlaceObjectPivotActor->SetActorLocation(NewLocation);
		}
	}
	
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

void UFactoryPlacementComponent::ProcessPlacementAction()
{
	switch (CurrentPlacementMode)
	{
	case EPlacementMode::PlaceFromData:
	case EPlacementMode::Move:
		PlaceObject();
		break;
		
	case EPlacementMode::BeltPlace:
		HandleBeltPlacementClick();
		break;
		
	default:
		break;
	}
}

#pragma region 프리뷰 제어

void UFactoryPlacementComponent::SetPlaceFromDataPreview(UFactoryObjectData* Data)
{
	if (!Data) return;
	ClearAllPreviews();
	
	AFactoryPlacePreview* Preview = GetWorld()->SpawnActor<AFactoryPlacePreview>();
	Preview->InitPreview(Data);
	ActivePreviews.Add(Preview);
	
	CurrentPlacementMode = EPlacementMode::PlaceFromData;
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
	UFactoryPoolSubsystem* Pool = GetWorld()->GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
	for (auto Preview : ActivePreviews)
	{
		if (Preview)
		{
			if (Pool)
			{
				Pool->ReturnItemToPool(Preview);
			}
			else
			{
				Preview->Destroy();
			}
		}
	}
	ActivePreviews.Empty();
	
	if (PlaceObjectPivotActor)
	{
		PlaceObjectPivotActor->SetHidden(true);
	}
	PlaceObjectPivotGridSize = FIntPoint(1,1);
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
			if (!Preview || !Preview->GetObjectData()) continue;
			
			FActorSpawnParameters Params; 
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AFactoryPlaceObjectBase* NewActor = GetWorld()->SpawnActor<AFactoryPlaceObjectBase>(
				Preview->GetObjectData()->PlaceObjectBP, 
				Preview->GetActorLocation(), 
				Preview->GetActorRotation(), 
				Params);
			
			if (CurrentPlacementMode == EPlacementMode::BeltPlace)
			{
				if (!BeltData)
				{
					UE_LOG(LogTemp, Error, TEXT("BeltData is NOT assigned in PlacementComponent!"));
					return;
				}
				
				if (AFactoryBelt* Belt = Cast<AFactoryBelt>(NewActor))
				{
					if (AFactoryBeltPreview* BeltPreview = Cast<AFactoryBeltPreview>(Preview))
					{
						Belt->SetBeltType(BeltPreview->GetBeltType());
					}
				}
			}
		}
		ClearAllPreviews();
		if (CurrentPlacementMode != EPlacementMode::BeltPlace)
		{
			CurrentPlacementMode = EPlacementMode::None;
		}
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
	bIsWaitingDetermineBeltEnd = false;
	CurrentPlacementMode = EPlacementMode::None;
}


bool UFactoryPlacementComponent::ToggleBeltPlaceMode()
{
	if (CurrentPlacementMode == EPlacementMode::BeltPlace)
	{
		CancelPlaceObject();
		return false;
	}
	
	CurrentPlacementMode = EPlacementMode::BeltPlace;
	bIsWaitingDetermineBeltEnd = false;
	
	FVector NewLocation;
	if (!TryGetPointingGridLocation(NewLocation))
	{
		CurrentPlacementMode = EPlacementMode::None;
		return false;
	}
	
	ResetBeltGuidePreview();
	
	return true;
}

void UFactoryPlacementComponent::HandleBeltPlacementClick()
{
	FVector SnappedPointingLocation = FVector::ZeroVector;
	if (!TryGetPointingGridLocation(SnappedPointingLocation)) return;
	
	if (!bIsWaitingDetermineBeltEnd)
	{
		if (TryGetBeltStartData(SnappedPointingLocation, BeltStartPoint, BeltStartDir))
		{
			bIsWaitingDetermineBeltEnd = true;
			ClearAllPreviews();
		}
	}
	else
	{
		PlaceObject();
		bIsWaitingDetermineBeltEnd = false;
		ResetBeltGuidePreview();
	}
}

void UFactoryPlacementComponent::BeltPlacePreviewUpdate(TArray<FBeltPlacementData> BeltPlacementDatas)
{
	if (CurrentPlacementMode != EPlacementMode::BeltPlace || !bIsWaitingDetermineBeltEnd ) return;
	
	UFactoryPoolSubsystem* Pool = GetWorld()->GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
	if (!Pool) return;
	
	for (auto& Preview : ActivePreviews)
	{
		if (Preview)
		{
			Pool->ReturnItemToPool(Preview);
		}
	}
	ActivePreviews.Empty();
	
	for (const FBeltPlacementData& Data : BeltPlacementDatas)
	{
		FVector WorldLoc = GridToWorld(Data.GridPoint);
		
		AFactoryBeltPreview* BeltPreview = Pool->GetItemFromPool<AFactoryBeltPreview>(
			EFactoryPoolType::BeltPreview, 
			WorldLoc, 
			Data.Rotation
		);
		
		if (BeltPreview)
		{
			BeltPreview->InitPreview(BeltData);
			BeltPreview->SetActorEnableCollision(true);
			BeltPreview->SetBeltType(Data.Type);
			ActivePreviews.Add(BeltPreview);
		}
	}
}

void UFactoryPlacementComponent::ResetBeltGuidePreview()
{
	ClearAllPreviews();
	UFactoryPoolSubsystem* Pool = GetWorld()->GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
	if (Pool)
	{
		AFactoryBeltPreview* Guide = Pool->GetItemFromPool<AFactoryBeltPreview>(
			EFactoryPoolType::BeltPreview, FVector::ZeroVector, FRotator::ZeroRotator);
		if (Guide)
		{
			Guide->InitPreview(BeltData);
			Guide->SetActorEnableCollision(true);
			ActivePreviews.Add(Guide);
			StartObjectPlaceMode(); // 피벗에 붙여서 마우스를 따라가게 함
		}
	}
}

void UFactoryPlacementComponent::CalculatePlacementPivotCenterAndGridSize()
{
	if (CurrentPlacementMode == EPlacementMode::BeltPlace && ActivePreviews.IsValidIndex(0))
	{
		PlaceObjectPivotGridSize = FIntPoint(1, 1);
		PlaceObjectPivotActor->SetActorLocation(ActivePreviews[0]->GetActorLocation());
	}
	else if (CurrentPlacementMode == EPlacementMode::PlaceFromData && ActivePreviews.IsValidIndex(0))
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

bool UFactoryPlacementComponent::TryGetPointingGridLocation(FVector& OutResultVec) const
{
	if (CurrentPlacementMode != EPlacementMode::BeltPlace && ActivePreviews.Num() <= 0) return false;

	FHitResult HitResult;
	bool bHit = false;
	
	if (AFactoryPlayerController* Controller = CachedPlayerController.Get())
	{
		if (Controller->GetCurrentViewMode() == EFactoryViewModeType::NormalView)
		{
			FVector CameraLocation; FRotator CameraRotation;
			Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
			FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * MaxBuildTraceDistance);
        
			FCollisionQueryParams Params; 
			Params.AddIgnoredActor(Controller->GetPawn());
			bHit = GetWorld()->LineTraceSingleByChannel(
				HitResult, CameraLocation, TraceEnd, ECC_GameTraceChannel1, Params);
        
			if (!bHit)
			{
				bHit = GetWorld()->LineTraceSingleByChannel(
					HitResult, 
					TraceEnd, TraceEnd + (FVector::DownVector * MaxBuildTraceDistance),
					ECC_GameTraceChannel1, Params);
			}
		}
		else
		{
			bHit = Controller->GetHitResultUnderCursor(ECC_GameTraceChannel1, false, HitResult);
		}
	}
    
	if (bHit)
	{
		OutResultVec = CalculateSnappedLocation(HitResult.Location, PlaceObjectPivotGridSize);
	}
	
	return bHit;
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

TArray<FBeltPlacementData> UFactoryPlacementComponent::CalculateBeltPath(
	const FIntPoint& StartPoint, const FIntPoint& EndPoint, const FVector& StartPointDir) const
{
	TArray<FIntPoint> Points;
	TArray<FBeltPlacementData> OutBeltPath;
	FIntPoint CurrentPoint = StartPoint;
	
	Points.Add(CurrentPoint);
	
	// X축 우선 이동
	int32 StepX = FMath::Sign(EndPoint.X - CurrentPoint.X);
	while (CurrentPoint.X != EndPoint.X && StepX != 0)
	{
		CurrentPoint.X += StepX;
		Points.Add(CurrentPoint);
	}
	
	// Y축 이동
	int32 StepY = FMath::Sign(EndPoint.Y - CurrentPoint.Y);
	while (CurrentPoint.Y != EndPoint.Y && StepY != 0)
	{
		CurrentPoint.Y += StepY;
		Points.Add(CurrentPoint);
	}
	
	// 회전 밑 벨트 타입 처리
	for (int i = 0; i < Points.Num(); i++)
	{
		FBeltPlacementData PlacementData;
		PlacementData.GridPoint = Points[i];
		
		FVector InDir = (i == 0) ? 
			StartPointDir : FVector(Points[i] - Points[i - 1], 0.f).GetSafeNormal();
		FVector EndDir = (i < Points.Num() - 1) ? 
			FVector(Points[i + 1] - Points[i], 0.f).GetSafeNormal() : InDir;	// 일단 마지막은 직선으로 처리
		
		PlacementData.Type = DetermineBeltType(InDir, EndDir);
		PlacementData.Rotation = InDir.Rotation();	// BP는 모두 진입 방향 = 액터 회전값으로 설계됨
		OutBeltPath.Add(PlacementData);
	}
	
	return OutBeltPath;
}

EBeltType UFactoryPlacementComponent::DetermineBeltType(const FVector& StartDir, const FVector& EndDir) const
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

FIntPoint UFactoryPlacementComponent::WorldToGrid(const FVector& WorldLocation) const
{
	return FIntPoint(
		FMath::FloorToInt(WorldLocation.X / GridLength),
		FMath::FloorToInt(WorldLocation.Y / GridLength)
	);
}

FVector UFactoryPlacementComponent::GridToWorld(const FIntPoint& GridLocation, const float Height) const
{
	return FVector(
		(GridLocation.X * GridLength) + (GridLength * 0.5f),
		(GridLocation.Y * GridLength) + (GridLength * 0.5f),
		Height
	);
}

bool UFactoryPlacementComponent::TryGetBeltStartData(const FVector& PointingLocation, 
	FIntPoint& OutStartGrid, FVector& OutStartDir) const
{
	// TODO : AFactoryLogisticsObject가 현재 그리드에 있는지 검사 후 처리하는 로직 추가. collision 활용?
	
	int XArr[4] = {1,-1,0,0};
	int YArr[4] = {0,0,1,-1};
	FHitResult HitResult;
	FVector StartLocation = PointingLocation + FVector(0,0,5);	// 지면과 너무 붙는 현상 방지
	float TraceLength = GridLength * 1.5f;
	
	for (int i = 0; i < 4; i++)
	{
		FVector EndLocation = PointingLocation + FVector(XArr[i] * TraceLength,YArr[i] * TraceLength,0);
		
		if (GetWorld()->LineTraceSingleByChannel(
			HitResult, StartLocation, EndLocation, ECC_GameTraceChannel2))	// Port 검사 채널
		{
			if (UFactoryOutputPortComponent* OutputPortComponent = Cast<UFactoryOutputPortComponent>(HitResult.GetComponent()))
			{
				OutStartGrid = WorldToGrid(StartLocation);
				OutStartDir = OutputPortComponent->GetForwardVector();
				return true;
			}
		}
	}
	
	return false;
}


