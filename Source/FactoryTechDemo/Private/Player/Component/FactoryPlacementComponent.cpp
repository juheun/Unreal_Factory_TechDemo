// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/FactoryPlacementComponent.h"

#include "EnhancedInputComponent.h"
#include "Logistics/FactoryBelt.h"
#include "Logistics/FactoryBeltBridge.h"
#include "Logistics/FactoryInputPortComponent.h"
#include "Logistics/FactoryLogisticsObjectBase.h"
#include "Logistics/FactoryLogisticsTypes.h"
#include "Logistics/FactoryOutputPortComponent.h"
#include "Placement/FactoryBeltBridgePreview.h"
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
	
	// 유효성 검사 람다 생성
	auto CheckValidity = [&]() -> bool {
		for (AFactoryPlacePreview* Preview : ActivePreviews)
			if (Preview->UpdateOverlapValidity() == EOverlapValidityResult::Invalid) return false;
		return true;
	};
	
	// 마우스 위치로 피벗 스냅
	FVector NewLocation;
	if (TryGetPointingGridLocation(NewLocation))
	{
		if (CurrentPlacementMode == EPlacementMode::BeltPlace && bIsWaitingDetermineBeltEnd)
		{
			FIntPoint EndPoint = WorldToGrid(NewLocation);
          
			// 기본 경로로 프리뷰를 생성
			BeltPlacePreviewUpdate(CalculateBeltPath(BeltStartPoint, EndPoint, BeltStartDir, false));
			
			// 프리뷰 유효성 검사하여 만약 기본 경로가 Invalid라면
			if (!CheckValidity())
			{
				// 대안 경로로 다시 생성
				BeltPlacePreviewUpdate(CalculateBeltPath(BeltStartPoint, EndPoint, BeltStartDir, true));
			}
		}
		else
		{
			PlaceObjectPivotActor->SetActorLocation(NewLocation);
		}
	}
	
	// 유효성(충돌) 검사
	bool bGlobalValid = CheckValidity();
	
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
		if (Controller->GetHitResultUnderCursor(ECC_GameTraceChannel3, false, HitResult)) 
		{
			if (AFactoryLogisticsObjectBase* HitObject = Cast<AFactoryLogisticsObjectBase>(HitResult.GetActor()))
			{
				if (HitObject->IsA(AFactoryBelt::StaticClass()))
				{
					return;		// 기획상 벨트는 이동할 수 없게 방어
				}
				
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

bool UFactoryPlacementComponent::PlaceObject()
{
	// Invalid가 하나라도 있다면 배치 불가 처리
	if (ActivePreviews.Num() <= 0) return false;
	for (auto Preview : ActivePreviews)
	{
		if (Preview->UpdateOverlapValidity() == EOverlapValidityResult::Invalid) 
		{ 
			return false;
		}
	}
	
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
		
		// 가장 최신 Validity데이터 갱신
		EOverlapValidityResult ValidityResult = Preview->UpdateOverlapValidity();
		
		//만약 Skip이면 건너뜀
		if (ValidityResult == EOverlapValidityResult::Skip)
		{
			continue;
		}
		
		// 만약 Replace면 이전에 있는 객체를 선제적으로 없앰
		if (ValidityResult == EOverlapValidityResult::Replace)
		{
			TArray<AFactoryPlaceObjectBase*> OverlappedObjects = Preview->GetOverlappingPlaceObjects();
         
			for (AFactoryPlaceObjectBase* OldObj : OverlappedObjects)
			{
				OldObj->Retrieve();
			}
		}
		
		// 실제 객체 배치 시작
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
				// 벨트라면 벨트 타입 설정
				if (AFactoryBeltPreview* BeltPreview = Cast<AFactoryBeltPreview>(Preview))
				{
					Belt->SetBeltType(BeltPreview->GetBeltType());
				}
			}
			
			// 배치된 객체가 인벤토리에서 소모되어야 하는 객체라면 배치되었음을 방송함
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
	
	OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
	return true;
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
		FVector GlobalMin(FLT_MAX), GlobalMax(-FLT_MAX);
		for (const auto& Preview : ActivePreviews)
		{
			FVector Loc = Preview->GetActorLocation();
			FIntPoint GridSize = Preview->GetObjectData()->GridSize;
			
			int32 Yaw = FMath::RoundToInt(Preview->GetActorRotation().Yaw);		// 객체의 회전값 불러서 필요시 XY 반전
			FIntPoint RotatedGridSize = GridSize;
			if (FMath::Abs(Yaw) % 180 == 90)	// 회전된 상태
			{
				RotatedGridSize = FIntPoint(GridSize.Y, GridSize.X);
			}
			
			FVector Extent(RotatedGridSize.X * GridLength * 0.5f, RotatedGridSize.Y * GridLength * 0.5f, 0.f);
			FVector ObjectMin = Loc - Extent;
			FVector ObjectMax = Loc + Extent;
			
			GlobalMin = GlobalMin.ComponentMin(ObjectMin);
			GlobalMax = GlobalMax.ComponentMax(ObjectMax);
		}

		// 중앙 지점으로 피벗 이동
		FVector Center = (GlobalMin + GlobalMax) * 0.5f;
		PlaceObjectPivotActor->SetActorLocation(Center);

		// 전체 그리드 크기 계산
		int32 SizeX = FMath::RoundToInt((GlobalMax.X - GlobalMin.X) / GridLength);
		int32 SizeY = FMath::RoundToInt((GlobalMax.Y - GlobalMin.Y) / GridLength);
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
		if (ActivePreviews.IsEmpty()) return;
		
		// 마지막 벨트의 정보 저장
		AFactoryPlacePreview* LastPreview = ActivePreviews.Last();
		FIntPoint NextStartGrid = WorldToGrid(LastPreview->GetActorLocation());
		FVector NextStartDir;
		
		if (AFactoryBeltPreview* BeltPreview = Cast<AFactoryBeltPreview>(LastPreview))
		{
			NextStartDir = BeltPreview->GetBeltExitDirection();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UFactoryPlacementComponent : HandleBeltPlacementClick에서 AFactoryBeltPreview Cast 실패"));
			NextStartDir = LastPreview->GetActorForwardVector();
		}
		
		if (PlaceObject())
		{
			bIsWaitingDetermineBeltEnd = true;		// 계속해서 이어서 벨트 배치
			BeltStartPoint = NextStartGrid;
			BeltStartDir = NextStartDir;
		}
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
		const UFactoryObjectData* TargetData = Data.bIsBridge ? BeltBridgeData : BeltData;
		
		AFactoryPlacePreview* Preview = CreateAndInitPreview(TargetData, GridToWorld(Data.GridPoint), Data.Rotation);
		
		if (Data.bIsBridge)
		{
			ActivePreviews.Add(Preview);	// 브릿지는 회전이나 Type 설정 필요없음
		}
		else
		{
			if (AFactoryBeltPreview* BeltPreview = Cast<AFactoryBeltPreview>(Preview))
			{
				BeltPreview->SetBeltType(Data.Type);
				ActivePreviews.Add(BeltPreview);
			}
		}
	}
}

void UFactoryPlacementComponent::ResetBeltGuidePreview()
{
	SetupSinglePreview(BeltData, EPlacementMode::BeltPlace);
	bIsWaitingDetermineBeltEnd = false;
}

TArray<FBeltPlacementData> UFactoryPlacementComponent::CalculateBeltPath(
	const FIntPoint& StartPoint, const FIntPoint& EndPoint, const FVector& StartPointDir, bool bAlternativeRoute) const
{
	TArray<FIntPoint> Points;
    Points.Add(StartPoint);

    if (StartPoint != EndPoint)
    {
        // 지정한 축(X 또는 Y)으로 목표 지점까지 한 칸씩 배열에 추가하며 이동
        auto MoveToX = [&](int32 TargetX) {
            int32 Step = FMath::Sign(TargetX - Points.Last().X);
            while (Points.Last().X != TargetX) {
                Points.Add(FIntPoint(Points.Last().X + Step, Points.Last().Y));
            }
        };
        auto MoveToY = [&](int32 TargetY) {
            int32 Step = FMath::Sign(TargetY - Points.Last().Y);
            while (Points.Last().Y != TargetY) {
                Points.Add(FIntPoint(Points.Last().X, Points.Last().Y + Step));
            }
        };

        FIntPoint StartDirInt(FMath::RoundToInt(StartPointDir.X), FMath::RoundToInt(StartPointDir.Y));
        bool bIsXDir = (StartDirInt.X != 0);

        // 목적지가 등 뒤에 있는지 판별
        bool bTargetIsBehind = false;
        if (bIsXDir && FMath::Sign(EndPoint.X - StartPoint.X) == -StartDirInt.X) bTargetIsBehind = true;
        if (!bIsXDir && FMath::Sign(EndPoint.Y - StartPoint.Y) == -StartDirInt.Y) bTargetIsBehind = true;

        if (!bTargetIsBehind)
        {
            // [정상 L자 라우팅]: 목표가 앞이나 옆에 있으면, 무조건 내가 바라보는 방향(직진)을 먼저 이동
            if (bIsXDir) { MoveToX(EndPoint.X); MoveToY(EndPoint.Y); }
            else         { MoveToY(EndPoint.Y); MoveToX(EndPoint.X); }
        }
        else
        {
            // [회피 U/L자 라우팅]: 목표가 등 뒤에 있으면, 무조건 좌/우 측면으로 먼저 이동
            int32 AltSign = bAlternativeRoute ? -1 : 1; // 충돌 시 반대편으로 꺾기 위한 배수

            if (bIsXDir)
            {
                if (StartPoint.Y == EndPoint.Y) // 목표가 정확히 일직선 뒤에 있을 때
                {
                    MoveToY(StartPoint.Y + AltSign); // 옆으로 1칸 회피
                    MoveToX(EndPoint.X);             // 뒤로 이동
                    MoveToY(EndPoint.Y);             // 다시 원래 라인으로 복귀
                }
                else
                {
                    MoveToY(EndPoint.Y); // 그냥 목표의 Y축으로 바로 꺾어서 빠져나감
                    MoveToX(EndPoint.X); // 그 다음 뒤로 이동
                }
            }
            else // 바라보는 방향이 Y축일 때
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
	
    TArray<FBeltPlacementData> OutBeltPath;
    for (int i = 0; i < Points.Num(); i++)
    {
    	// 만약 벨트 브릿지가 있다면 해당 위치는 프리뷰를 생성하지 않도록 변경
    	FVector CurrentLoc = GridToWorld(Points[i]);
    	FHitResult TileHit;
    	FVector TraceStart = CurrentLoc + FVector(0, 0, 50.f);
    	FVector TraceEnd = CurrentLoc - FVector(0, 0, 50.f);
    	FCollisionObjectQueryParams ObjectParams;
    	ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel3);
    	
    	bool bHitObject = GetWorld()->LineTraceSingleByObjectType(TileHit, TraceStart, TraceEnd, ObjectParams);
    	AActor* HitActor = bHitObject ? TileHit.GetActor() : nullptr;
    	
    	if (HitActor && HitActor->IsA<AFactoryBeltBridge>())
		{
			continue; 
		}
    	
        FBeltPlacementData PlacementData;
    	PlacementData.GridPoint = Points[i];
       
        FVector InDir = (i == 0) ? 
          StartPointDir : FVector(Points[i] - Points[i - 1], 0.f).GetSafeNormal();
    	
	    FVector EndDir;
    	bool bIsCrossingOrthogonalBelt = false;		// 브릿지 생성 플래그
    	
	    if (i < Points.Num() - 1)
	    {
    		// 중간 벨트들은 다음 벨트를 향해 방향을 잡음
    		EndDir = FVector(Points[i + 1] - Points[i], 0.f).GetSafeNormal();
	    	
	    	// 브릿지 생성 감지 로직
	    	if (i > 0 && HitActor) 
	    	{
	    		if (AFactoryBelt* HitBelt = Cast<AFactoryBelt>(HitActor))
	    		{
	    			if (HitBelt->GetBeltType() == EBeltType::Straight)
	    			{
	    				FVector BeltDir = HitBelt->GetActorRotation().Vector().GetSafeNormal();
                       
	    				// 직교 검사
	    				if (FMath::Abs(FVector::DotProduct(BeltDir, InDir)) < 0.1f)
	    				{
	    					bIsCrossingOrthogonalBelt = true;
	    				}
	    			}
	    		}
	    	}
	    }
	    else
	    {
    		// 마지막 벨트. 주변 InputPort를 스캔
    		EndDir = InDir; // 기본값은 직진
	       
	    	bool bLastBeltOverwritten = false;
	    	
	    	// 마지막 벨트에 이미 벨트가 존재했었는지 확인
			if (HitActor)
			{
				if (AFactoryBelt* HitBelt = Cast<AFactoryBelt>(HitActor))
				{
					EndDir = HitBelt->GetBeltExitDirection();
					bLastBeltOverwritten = true; // 벨트를 찾았으므로 포트 스캔 통과
				}
			}
	    	
	    	// 마지막 벨트의 덮어쓰기가 없을때만 주변 포트 스캔
	    	if (!bLastBeltOverwritten)
	    	{
	    		int XArr[4] = {1, -1, 0, 0};
	    		int YArr[4] = {0, 0, 1, -1};
	    		FVector PortSearchLoc = FVector(CurrentLoc.X, CurrentLoc.Y, 5.f);		// 포트 감지위해 Z값 조정
	       
	    		for (int j = 0; j < 4; j++)
	    		{
	    			FVector SearchLoc = PortSearchLoc + FVector(XArr[j] * GridLength, YArr[j] * GridLength, 5.f);
	    			FHitResult PortHit;
	           
	    			// 내 주변에 InputPort가 있는지 트레이스
	    			if (GetWorld()->LineTraceSingleByChannel(PortHit, PortSearchLoc, SearchLoc, ECC_GameTraceChannel2))
	    			{
	    				if (UFactoryInputPortComponent* InputPort = Cast<UFactoryInputPortComponent>(PortHit.GetComponent()))
	    				{
	    					// 포트를 발견하면, 그 포트를 향하는 방향으로 EndDir을 강제 수정
	    					EndDir = InputPort->GetForwardVector();
	    					break; 
	    				}
	    			}
	    		}
	    	}
	    }
       
        PlacementData.Type = DetermineBeltType(InDir, EndDir);
        PlacementData.Rotation = InDir.Rotation();
    	PlacementData.bIsBridge = bIsCrossingOrthogonalBelt;
    	
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
	OutStartGrid = WorldToGrid(PointingLocation);
	FHitResult HitResult;
	
	// 현재 클릭한 그리드에 기존 벨트가 있는지 수직으로 검사 (벨트 덮어쓰기 및 연장용)
	FVector TraceStart = PointingLocation + FVector(0, 0, 50.f);
	FVector TraceEnd = PointingLocation - FVector(0, 0, 50.f);
	
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel3);	// PlaceObject 채널
	
	if (GetWorld()->LineTraceSingleByObjectType(HitResult, TraceStart, TraceEnd, ObjectParams))
	{
		if (AFactoryBelt* HitBelt = Cast<AFactoryBelt>(HitResult.GetActor()))
		{
			OutStartDir = HitBelt->GetActorForwardVector();
			return true;
		}
	}
	
	// 설비의 Outport찾기
	int XArr[4] = {1,-1,0,0};
	int YArr[4] = {0,0,1,-1};
	FVector StartLocation = PointingLocation + FVector(0,0,5);	// 지면과 너무 붙는 현상 방지
	float TraceLength = GridLength * 1.f;
	
	for (int i = 0; i < 4; i++)
	{
		FVector SearchEnd = StartLocation + FVector(XArr[i] * TraceLength,YArr[i] * TraceLength,0);
		
		if (GetWorld()->LineTraceSingleByChannel(
			HitResult, StartLocation, SearchEnd, ECC_GameTraceChannel2))	// Port 검사 채널
		{
			if (UFactoryOutputPortComponent* OutputPortComponent = Cast<UFactoryOutputPortComponent>(HitResult.GetComponent()))
			{
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
	else if (Data == BeltBridgeData)
	{
		Preview = Pool->GetItemFromPool<AFactoryBeltBridgePreview>(AFactoryBeltBridgePreview::StaticClass(), Loc, Rot);
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

