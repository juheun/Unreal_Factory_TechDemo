// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/FactoryPlacementComponent.h"

#include "EnhancedInputComponent.h"
#include "Logistics/FactoryBelt.h"
#include "Logistics/FactoryLogisticsObjectBase.h"
#include "Logistics/FactoryLogisticsTypes.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Placement/FactoryBeltPreview.h"
#include "Placement/FactoryObjectData.h"
#include "Placement/FactoryPlaceObjectBase.h"
#include "Placement/FactoryPlacePreview.h"
#include "Player/FactoryPlayerController.h"
#include "Player/Input/FactoryInputConfig.h"
#include "Settings/FactoryDeveloperSettings.h"
#include "Subsystems/FactoryPoolSubsystem.h"


UFactoryPlacementComponent::UFactoryPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

#pragma region 엔진 라이프 사이클 및 컨트롤러 소통

void UFactoryPlacementComponent::SetUpInputComponent(UEnhancedInputComponent* PlayerInputComp,
	const UFactoryInputConfig* InputConfig)
{
	PlayerInputComp->BindAction(InputConfig->PlaceObjectAction, ETriggerEvent::Started, this, &UFactoryPlacementComponent::ProcessClickAction);
	PlayerInputComp->BindAction(InputConfig->PlaceObjectCancelAction, ETriggerEvent::Started, this, &UFactoryPlacementComponent::CancelPlaceObject);
	PlayerInputComp->BindAction(InputConfig->PlaceObjectRotateAction, ETriggerEvent::Started, this, &UFactoryPlacementComponent::RotatePlacementPreview);
	PlayerInputComp->BindAction(InputConfig->ToggleBeltPlaceModeAction, ETriggerEvent::Started, this, &UFactoryPlacementComponent::ToggleBeltPlaceMode);
	PlayerInputComp->BindAction(InputConfig->ToggleRetrieveModeAction, ETriggerEvent::Started, this, &UFactoryPlacementComponent::ToggleRetrieveMode);
	PlayerInputComp->BindAction(InputConfig->EnterMoveModeAction, ETriggerEvent::Triggered, this, &UFactoryPlacementComponent::TryEnterMoveMode);
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

#pragma endregion

#pragma region 인터페이스, 모드 토글

void UFactoryPlacementComponent::ProcessClickAction()
{
	switch (CurrentPlacementMode)
	{
	case EPlacementMode::PlaceFromInventory:
	case EPlacementMode::Move:
		PlaceObject();
		break;
		
	case EPlacementMode::BeltPlace:
		HandleBeltPlacementClick();
		break;
		
	case EPlacementMode::Retrieve:
		break;
		
	default:
		break;
	}
}


void UFactoryPlacementComponent::ToggleBeltPlaceMode()
{
	if (CurrentPlacementMode == EPlacementMode::BeltPlace)
	{
		CancelPlaceObject();
		CurrentPlacementMode = EPlacementMode::None;
	}
	else if (CurrentPlacementMode != EPlacementMode::None)
	{
		return;	// 벨트 토글을 받을 상황이 아니므로 입력을 무시
	}
	else
	{
		// 먼저 벨트모드로 변경해야 TryGetPointingGridLocation의 가드에 걸리지 않음
		CurrentPlacementMode = EPlacementMode::BeltPlace;
		bIsWaitingDetermineBeltEnd = false;
       
		FVector NewLocation;
		if (!TryGetPointingGridLocation(NewLocation))
		{
			CurrentPlacementMode = EPlacementMode::None;
			OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
			return;
		}
       
		ResetBeltGuidePreview();
	}
	OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
}

void UFactoryPlacementComponent::ToggleRetrieveMode()
{
	if (CurrentPlacementMode == EPlacementMode::Retrieve)
	{
		CurrentPlacementMode = EPlacementMode::None;
	}
	else if (CurrentPlacementMode != EPlacementMode::None)
	{
		return;	// 수납모드 토글을 받을 상황이 아니므로 입력을 무시
	}
	else
	{
		CancelPlaceObject();	// 기존 배치 모드 취소
		CurrentPlacementMode = EPlacementMode::Retrieve;
	}
	
	OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
}

void UFactoryPlacementComponent::TryEnterMoveMode()
{
	if (CurrentPlacementMode != EPlacementMode::None) return;

	if (AFactoryPlayerController* Controller = CachedPlayerController.Get())
	{
		if (Controller->GetCurrentViewMode() != EFactoryViewModeType::TopView) return;

		FHitResult HitResult;
		if (Controller->GetHitResultUnderCursor(ECC_Visibility, false, HitResult)) 
		{
			if (AFactoryLogisticsObjectBase* HitObject = Cast<AFactoryLogisticsObjectBase>(HitResult.GetActor()))
			{
				SelectObject(HitObject);
				SetMoveObjectToPreviews();
			}
		}
	}
}


#pragma endregion

#pragma region 배치 및 프리뷰 제어

void UFactoryPlacementComponent::SetupSinglePreview(UFactoryObjectData* Data, EPlacementMode Mode)
{
	if (!Data) return;
	ClearAllPreviews();
	
	AFactoryPlacePreview* Preview = CreateAndInitPreview(Data);
	if (Preview)
	{
		ActivePreviews.Add(Preview);
		CurrentPlacementMode = Mode;
		OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
		StartObjectPlaceMode();
	}
}

void UFactoryPlacementComponent::SetPlaceFromDataPreview(UFactoryObjectData* Data)
{
	SetupSinglePreview(Data, EPlacementMode::PlaceFromInventory);
}

void UFactoryPlacementComponent::SetMoveObjectToPreviews()
{
	if (SelectedLogisticsObjectBases.Num() == 0) return;
	ClearAllPreviews();
	
	for (auto& LogisticsObjectBase : SelectedLogisticsObjectBases)
	{
		const UFactoryObjectData* Data = LogisticsObjectBase->GetObjectData();
		AFactoryPlacePreview* Preview = CreateAndInitPreview(
			Data, LogisticsObjectBase->GetActorLocation(), LogisticsObjectBase->GetActorRotation());
		
		if (Preview)
		{
			if (Preview->IsA(AFactoryBeltPreview::StaticClass()))
			{
				AFactoryBeltPreview* BeltPreview = Cast<AFactoryBeltPreview>(Preview);
				BeltPreview->SetBeltType(Cast<AFactoryBelt>(LogisticsObjectBase)->GetBeltType());
			}
			
			ActivePreviews.Add(Preview);
		}
		
		LogisticsObjectBase->SetActorHiddenInGame(true);
		LogisticsObjectBase->SetActorEnableCollision(false);
	}
	
	CurrentPlacementMode = EPlacementMode::Move;
	OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
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

void UFactoryPlacementComponent::RotatePlacementPreview()
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
		if (CurrentPlacementMode == EPlacementMode::Move)
		{
			for (auto& LogisticsObjectBase : SelectedLogisticsObjectBases)
			{
				LogisticsObjectBase->Destroy();
			}
			SelectedLogisticsObjectBases.Empty();
		}
		
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
			
			if (NewActor)
			{
				NewActor->InitObject(Preview->GetObjectData());
				if (AFactoryBelt* Belt = Cast<AFactoryBelt>(NewActor))
				{
					if (!BeltData)
					{
						UE_LOG(LogTemp, Error, TEXT("BeltData is NOT assigned in PlacementComponent!"));
						return;
					}
					if (AFactoryBeltPreview* BeltPreview = Cast<AFactoryBeltPreview>(Preview))
					{
						Belt->SetBeltType(BeltPreview->GetBeltType());
					}
				}
				
				if (CurrentPlacementMode == EPlacementMode::PlaceFromInventory && NewActor->GetObjectData()->bRefundItemOnDestroy)
				{
					OnObjectPlacedFromInventorySignature.Broadcast(NewActor->GetObjectData()->RepresentingItemData, 1);
				}
			}
		}
		ClearAllPreviews();
		if (CurrentPlacementMode != EPlacementMode::BeltPlace)
		{
			CurrentPlacementMode = EPlacementMode::None;
		}
	}
	
	OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
}

void UFactoryPlacementComponent::CancelPlaceObject()
{
	// 이동 모드 취소 시 숨겼던 원본을 다시 복구
	if (CurrentPlacementMode == EPlacementMode::Move)
	{
		for (auto& Obj : SelectedLogisticsObjectBases)
		{
			if (Obj)
			{
				Obj->SetActorHiddenInGame(false);
				Obj->SetActorEnableCollision(true);
			}
		}
		// TODO : 상황에 따라 선택 배열이 남아있어야 할수도 있는 부분 체크
		ClearObject(); // 선택 배열 비우기
	}
	
	ClearAllPreviews();
	bIsWaitingDetermineBeltEnd = false;
	CurrentPlacementMode = EPlacementMode::None;
	OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
}

void UFactoryPlacementComponent::CalculatePlacementPivotCenterAndGridSize()
{
	if (CurrentPlacementMode == EPlacementMode::BeltPlace && ActivePreviews.IsValidIndex(0))
	{
		PlaceObjectPivotGridSize = FIntPoint(1, 1);
		PlaceObjectPivotActor->SetActorLocation(ActivePreviews[0]->GetActorLocation());
	}
	else if (CurrentPlacementMode == EPlacementMode::PlaceFromInventory && ActivePreviews.IsValidIndex(0))
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

#pragma endregion 

#pragma region 벨트 제어

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
	
	UFactoryPoolSubsystem* Pool = GetPool();
	if (!Pool) return;
	
	for (auto& Preview : ActivePreviews)
	{
		if (Preview) Pool->ReturnItemToPool(Preview);
	}
	ActivePreviews.Empty();
	
	for (const FBeltPlacementData& Data : BeltPlacementDatas)
	{
		AFactoryPlacePreview* Preview = CreateAndInitPreview(BeltData, GridToWorld(Data.GridPoint), Data.Rotation);
		
		if (AFactoryBeltPreview* BeltPreview = Cast<AFactoryBeltPreview>(Preview))
		{
			BeltPreview->SetBeltType(Data.Type);
			ActivePreviews.Add(BeltPreview);
		}
	}
}

void UFactoryPlacementComponent::ResetBeltGuidePreview()
{
	SetupSinglePreview(BeltData, EPlacementMode::BeltPlace);
	bIsWaitingDetermineBeltEnd = false;
}

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

#pragma endregion

#pragma region 객체 선택

void UFactoryPlacementComponent::SelectObject(AFactoryLogisticsObjectBase* TargetObject)
{
	if (TargetObject)
	{
		SelectedLogisticsObjectBases.Add(TargetObject);
	}
	//TODO : 선택된 객체들에 대한 시각적 피드백(예: 하이라이트) 구현
}

void UFactoryPlacementComponent::DeselectObject(AFactoryLogisticsObjectBase* TargetObject)
{
	if (TargetObject)
	{
		SelectedLogisticsObjectBases.Remove(TargetObject);
	}
	//TODO : 선택 해제된 객체에 대한 시각적 피드백 제거 구현
}

void UFactoryPlacementComponent::ClearObject()
{
	for (auto Object : SelectedLogisticsObjectBases)
	{
		//TODO : 선택 해제된 객체에 대한 시각적 피드백 제거 구현
	}
	SelectedLogisticsObjectBases.Empty();
}

#pragma endregion

#pragma region 유틸리티

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

UFactoryPoolSubsystem* UFactoryPlacementComponent::GetPool() const
{
	return GetWorld()->GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
}

AFactoryPlacePreview* UFactoryPlacementComponent::CreateAndInitPreview(
	const UFactoryObjectData* Data, const FVector& Loc, const FRotator& Rot) const
{
	if (!Data) return nullptr;
	
	AFactoryPlacePreview* Preview = nullptr;
	UFactoryPoolSubsystem* Pool = GetPool();
	if (!Pool) return nullptr;
	
	if (Data == BeltData)
	{
		Preview = Pool->GetItemFromPool<AFactoryBeltPreview>(EFactoryPoolType::BeltPreview, Loc, Rot);
	}
	else
	{
		Preview = Pool->GetItemFromPool<AFactoryPlacePreview>(AFactoryPlacePreview::StaticClass(), Loc, Rot);
	}
	
	if (Preview)
	{
		Preview->InitPreview(Data);
		Preview->SetActorEnableCollision(true);
	}
	
	return Preview;
}

#pragma endregion

