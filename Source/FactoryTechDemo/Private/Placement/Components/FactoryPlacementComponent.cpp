// Fill out your copyright notice in the Description page of Project Settings.


#include "Placement/Components/FactoryPlacementComponent.h"

#include "EnhancedInputComponent.h"
#include "Logistics/Belts/FactoryBelt.h"
#include "Logistics/Machines/FactoryLogisticsObjectBase.h"
#include "Logistics/FactoryLogisticsTypes.h"
#include "Placement/Previews/FactoryBeltBridgePreview.h"
#include "Placement/Previews/FactoryBeltPreview.h"
#include "Placement/FactoryObjectData.h"
#include "Placement/FactoryPlaceObjectBase.h"
#include "Placement/Previews/FactoryPlacePreview.h"
#include "Player/Input/FactoryPlayerController.h"
#include "Player/Input/FactoryInputConfig.h"
#include "Core/FactoryDeveloperSettings.h"
#include "Libraries/FactoryGridMathLibrary.h"
#include "Placement/Components/FactoryBeltBuilderComponent.h"
#include "Subsystems/FactoryPoolSubsystem.h"
#include "UI/PlayerContext/FactoryDragSelectionWidget.h"


UFactoryPlacementComponent::UFactoryPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	BeltBuilder = CreateDefaultSubobject<UFactoryBeltBuilderComponent>(TEXT("BeltBuilder"));
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
	PlayerInputComp->BindAction(InputConfig->ToggleMultipleControlModeAction, ETriggerEvent::Started, this, &UFactoryPlacementComponent::ToggleMultipleControlMode);
	
	PlayerInputComp->BindAction(InputConfig->MultipleControlAddAction, ETriggerEvent::Started, this, &UFactoryPlacementComponent::OnMultipleControlAddStarted);
	PlayerInputComp->BindAction(InputConfig->MultipleControlAddAction, ETriggerEvent::Triggered, this, &UFactoryPlacementComponent::UpdateDragSelectionBox);
	PlayerInputComp->BindAction(InputConfig->MultipleControlAddAction, ETriggerEvent::Completed, this, &UFactoryPlacementComponent::OnMultipleControlAddCompleted);

	PlayerInputComp->BindAction(InputConfig->MultipleControlRemoveAction, ETriggerEvent::Started, this, &UFactoryPlacementComponent::OnMultipleControlRemoveStarted);
	PlayerInputComp->BindAction(InputConfig->MultipleControlRemoveAction, ETriggerEvent::Triggered, this, &UFactoryPlacementComponent::UpdateDragSelectionBox);
	PlayerInputComp->BindAction(InputConfig->MultipleControlRemoveAction, ETriggerEvent::Completed, this, &UFactoryPlacementComponent::OnMultipleControlRemoveCompleted);
	
	PlayerInputComp->BindAction(InputConfig->MultipleControlMoveAction, ETriggerEvent::Started, this, &UFactoryPlacementComponent::OnMultipleControlMove);
	PlayerInputComp->BindAction(InputConfig->MultipleControlRetrieveAction, ETriggerEvent::Started, this, &UFactoryPlacementComponent::OnMultipleControlRetrieve);
}

void UFactoryPlacementComponent::BeginPlay()
{
	Super::BeginPlay();
    
    if (const UFactoryDeveloperSettings* Settings = GetDefault<UFactoryDeveloperSettings>())
    {
        GridLength = Settings->GetGridLength();
    }
	
    CachedPlayerController = Cast<AFactoryPlayerController>(GetOwner());
	
	if (CachedPlayerController.Get() && DragSelectionWidgetBP)
	{
		DragSelectionWidget = CreateWidget<UFactoryDragSelectionWidget>(CachedPlayerController.Get(), DragSelectionWidgetBP);
		if (DragSelectionWidget)
		{
			DragSelectionWidget->AddToViewport();
			DragSelectionWidget->StopDrag(); // 처음에 위젯 숨기기
		}
	}
	
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
	
	// 다중 제어모드에서는 충돌 검사를 할 필요가 없음. 그냥 유효하다고 치고 복잡한 검사 스킵
	if (CurrentPlacementMode == EPlacementMode::MultipleControl)
	{
		for (AFactoryPlacePreview* Preview : ActivePreviews)
		{
			Preview->SetVisualValidity(true); 
		}
		return;
	}
	
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
		if (CurrentPlacementMode == EPlacementMode::BeltPlace && BeltBuilder)
		{
			if (BeltBuilder->IsWaitingForEndPoint())
			{
				TArray<FBeltPlacementData> PathData;
				
				if (BeltBuilder->GetPreviewPathData(NewLocation, GridLength, false, PathData))
				{
					RenderBeltPreviews(PathData);
					
					// 만약 첫번째 루트로 검사 후 실패시 대안경로로 생성
					if (!CheckValidity())
					{
						if (BeltBuilder->GetPreviewPathData(NewLocation, GridLength, true, PathData))
						{
							RenderBeltPreviews(PathData);
						}
					}
				}
			}
			else
			{
				FVector FinalLocation = BeltBuilder->GetSnappedStartLocation(NewLocation, GridLength);
				PlaceObjectPivotActor->SetActorLocation(FinalLocation);
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
		FVector PointingLoc;
		if (TryGetPointingGridLocation(PointingLoc))
		{
			if (BeltBuilder && BeltBuilder->ProcessClick(PointingLoc, GridLength))
			{
				// 현재 마우스 위치 다시 구해서 넘겨주기
				FVector NextStartLoc = PointingLoc;
				if (ActivePreviews.Num() > 0 && ActivePreviews.Last())
				{
					NextStartLoc = ActivePreviews.Last()->GetActorLocation();
				}

				if (PlaceObject())
				{
					// 스폰 성공 시, 방금 캐싱한 끝점을 다시 클릭한 것처럼 만들어서 연속 짓기 돌입
					BeltBuilder->ResetBuilderState();
					BeltBuilder->ProcessClick(NextStartLoc, GridLength); 
				}
			}
		}
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
		if (BeltBuilder) BeltBuilder->ResetBuilderState();
       
		FVector NewLocation;
		if (!TryGetPointingGridLocation(NewLocation))
		{
			CurrentPlacementMode = EPlacementMode::None;
			OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
			return;
		}
       
		if (BeltBuilder) SetupSinglePreview(BeltBuilder->GetBeltData(), EPlacementMode::BeltPlace);
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

void UFactoryPlacementComponent::ToggleMultipleControlMode()
{
	if (CurrentPlacementMode == EPlacementMode::None)
	{
		CurrentPlacementMode = EPlacementMode::MultipleControl;
		ClearSelectedObject();
		OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
	}
	else if (CurrentPlacementMode == EPlacementMode::MultipleControl)
	{
		CancelPlaceObject();
	}
	
	if (DragSelectionWidget)
	{
		DragSelectionWidget->StopDrag();
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
		PlaceObjectPivotActor->SetActorRotation(FRotator::ZeroRotator);
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
	
	CalculatePlacementPivotCenterAndGridSize();
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
	if (CurrentPlacementMode == EPlacementMode::Move || CurrentPlacementMode == EPlacementMode::MultipleControl)
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
		ClearSelectedObject(); // 선택 배열 비우기
	}
	
	if (DragSelectionWidget)
	{
		DragSelectionWidget->StopDrag();
	}
	
	if (BeltBuilder) BeltBuilder->ResetBuilderState();
	ClearAllPreviews();
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
			int32 Yaw = FMath::RoundToInt(PlaceObjectPivotActor->GetActorRotation().Yaw);
			FIntPoint RotatedGridSize = Data->GridSize;
			if (FMath::Abs(Yaw) % 180 == 90)
			{
				RotatedGridSize = FIntPoint(Data->GridSize.Y, Data->GridSize.X);
			}
			PlaceObjectPivotGridSize = RotatedGridSize;
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

void UFactoryPlacementComponent::RenderBeltPreviews(const TArray<FBeltPlacementData>& BeltPlacementDatas)
{
	if (CurrentPlacementMode != EPlacementMode::BeltPlace || !BeltBuilder ) return;
	
	UFactoryPoolSubsystem* Pool = GetPool();
	if (!Pool) return;
	
	for (auto& Preview : ActivePreviews)
	{
		if (Preview) Pool->ReturnItemToPool(Preview);
	}
	ActivePreviews.Empty();
	
	for (const FBeltPlacementData& Data : BeltPlacementDatas)
	{
		const UFactoryObjectData* TargetData = Data.bIsBridge ? BeltBuilder->GetBeltBridgeData() : BeltBuilder->GetBeltData();
		
		AFactoryPlacePreview* Preview = CreateAndInitPreview(
			TargetData, UFactoryGridMathLibrary::GridToWorld(Data.GridPoint, GridLength), Data.Rotation);
		
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

#pragma endregion

#pragma region 객체 선택

void UFactoryPlacementComponent::SelectObject(AFactoryLogisticsObjectBase* TargetObject)
{
	if (!TargetObject || SelectedLogisticsObjectBases.Contains(TargetObject)) return;

	SelectedLogisticsObjectBases.Add(TargetObject);
	TargetObject->SetActorHiddenInGame(true);

	// 프리뷰 생성
	AFactoryPlacePreview* Preview = CreateAndInitPreview(TargetObject->GetObjectData(), TargetObject->GetActorLocation(), TargetObject->GetActorRotation());
	if (Preview)
	{
		Preview->InitPreview(TargetObject->GetObjectData());
		if (Preview->IsA(AFactoryBeltPreview::StaticClass()))
		{
			Cast<AFactoryBeltPreview>(Preview)->SetBeltType(Cast<AFactoryBelt>(TargetObject)->GetBeltType());
		}
		ActivePreviews.Add(Preview);
	}
}

void UFactoryPlacementComponent::DeselectObject(AFactoryLogisticsObjectBase* TargetObject)
{
	if (!TargetObject || !SelectedLogisticsObjectBases.Contains(TargetObject)) return;

	SelectedLogisticsObjectBases.Remove(TargetObject);
	TargetObject->SetActorHiddenInGame(false);

	// 해당 위치에 생성되었던 프리뷰 찾아서 파괴
	for (int i = ActivePreviews.Num() - 1; i >= 0; i--)
	{
		if (ActivePreviews[i]->GetActorLocation().Equals(TargetObject->GetActorLocation(), 1.f))
		{
			UFactoryPoolSubsystem* Pool = GetPool();
			if (Pool) Pool->ReturnItemToPool(ActivePreviews[i]);
			else ActivePreviews[i]->Destroy();
			
			ActivePreviews.RemoveAt(i);
			break;
		}
	}
}

void UFactoryPlacementComponent::ClearSelectedObject()
{
	// 배열을 순회하며 원본 복구 및 프리뷰 제거
	for (int i = SelectedLogisticsObjectBases.Num() - 1; i >= 0; i--)
	{
		DeselectObject(SelectedLogisticsObjectBases[i]);
	}
	SelectedLogisticsObjectBases.Empty();
}

void UFactoryPlacementComponent::OnMultipleControlAddStarted()
{
	BeginDragSelection(false);
}

void UFactoryPlacementComponent::OnMultipleControlAddCompleted()
{
	EndDragSelection(false);
}

void UFactoryPlacementComponent::OnMultipleControlRemoveStarted()
{
	BeginDragSelection(true);
}

void UFactoryPlacementComponent::OnMultipleControlRemoveCompleted()
{
	EndDragSelection(true);
}

void UFactoryPlacementComponent::BeginDragSelection(bool bIsRemove)
{
	if (CurrentPlacementMode != EPlacementMode::MultipleControl) return;
	FVector HitLocation;
	if (TryGetPointingGridLocation(HitLocation))
	{
		SelectionDragStartPoint = UFactoryGridMathLibrary::WorldToGrid(HitLocation, GridLength);
		bIsDraggingSelection = true;
		bIsRemoveDrag = bIsRemove;
        
		PreDragSelection.Empty();
		PreDragSelection.Append(SelectedLogisticsObjectBases);
		
		if (DragSelectionWidget)
		{
			DragSelectionWidget->StartDrag(HitLocation);
		}
	}
}

void UFactoryPlacementComponent::EndDragSelection(bool bIsRemove)
{
	if (!bIsDraggingSelection) return;
	bIsDraggingSelection = false;
	
	FVector CurrentLocation;
	if (TryGetPointingGridLocation(CurrentLocation))
	{
		if (SelectionDragStartPoint == UFactoryGridMathLibrary::WorldToGrid(CurrentLocation, GridLength))	// StartGrid = EndGrid라면 클릭
		{
			if (!bIsRemove)
			{
				SelectConnectedBeltLine();
			}
			else
			{
				ToggleMultipleControlMode();	// 단순 우클릭 처리. 선택모드 종료
			}
		}
	}
	// 드래그 선택 위젯 끄기
	if (DragSelectionWidget)
	{
		DragSelectionWidget->StopDrag();
	}
}

void UFactoryPlacementComponent::UpdateDragSelectionBox()
{
	if (!bIsDraggingSelection) return;
	
	FVector CurrentLocation;
	if (!TryGetPointingGridLocation(CurrentLocation)) return;
	
	// 드래그 박스 안 겹치는 객체 도출
	FIntPoint CurrentGrid = UFactoryGridMathLibrary::WorldToGrid(CurrentLocation, GridLength);
	TArray<AFactoryLogisticsObjectBase*> OverlappedObjects = 
		UFactoryGridMathLibrary::GetFacilitiesInGridBox(this, SelectionDragStartPoint, CurrentGrid, GridLength);
	
	// 
	TSet<AFactoryLogisticsObjectBase*> DesiredSelection = PreDragSelection;
	for (auto* HitObj : OverlappedObjects)
	{
		if (bIsRemoveDrag) DesiredSelection.Remove(HitObj);
		else DesiredSelection.Add(HitObj);
	}
	// 없애야할 객체 Deselect
	for (int i = SelectedLogisticsObjectBases.Num() - 1; i >= 0; i--)
	{
		if (!DesiredSelection.Contains(SelectedLogisticsObjectBases[i]))
		{
			DeselectObject(SelectedLogisticsObjectBases[i]);
		}
	}
	// 추가해야 할 객체 Select
	for (auto* Obj : DesiredSelection)
	{
		SelectObject(Obj);
	}
	
	// 드래그 선택 위젯 업데이트
	if (DragSelectionWidget && CachedPlayerController.IsValid())
	{
		float MouseX, MouseY;
		if (CachedPlayerController.Get()->GetMousePosition(MouseX, MouseY))
		{
			DragSelectionWidget->UpdateDrag(CachedPlayerController.Get(), FVector2D(MouseX, MouseY));
		}
	}
}

void UFactoryPlacementComponent::SelectConnectedBeltLine()
{
	FVector CurrentLocation;
	if (TryGetPointingGridLocation(CurrentLocation))
	{
		if (AActor* HitActor = UFactoryGridMathLibrary::GetFacilityAtGrid(this, CurrentLocation, GridLength))
		{
			if (AFactoryBelt* HitBelt = Cast<AFactoryBelt>(HitActor))
			{
				TSet<AFactoryBelt*> ConnectedBelts = HitBelt->GetConnectedBeltLine();
				for (AFactoryBelt* Belt : ConnectedBelts)
				{
					SelectObject(Belt);
				}
			}
		}
	}
}

void UFactoryPlacementComponent::OnMultipleControlMove()
{
	if (CurrentPlacementMode == EPlacementMode::MultipleControl && SelectedLogisticsObjectBases.Num() > 0)
	{
		SetMoveObjectToPreviews(); 
	}
}

void UFactoryPlacementComponent::OnMultipleControlRetrieve()
{
	if (CurrentPlacementMode == EPlacementMode::MultipleControl && SelectedLogisticsObjectBases.Num() > 0)
	{
		// 원본 배열이 순회 중 삭제/수정될 수 있으므로 임시 복사
		TArray<AFactoryLogisticsObjectBase*> ObjectsToRetrieve = SelectedLogisticsObjectBases;
		
		ClearSelectedObject(); // 배열 비우고 프리뷰 싹 지우기
		
		for (AFactoryLogisticsObjectBase* Obj : ObjectsToRetrieve)
		{
			if (Obj) Obj->Retrieve();
		}
	}
}

#pragma endregion

#pragma region 유틸리티

bool UFactoryPlacementComponent::TryGetPointingGridLocation(FVector& OutResultVec) const
{
	if (CurrentPlacementMode == EPlacementMode::None) return false;
	
	if (CurrentPlacementMode != EPlacementMode::BeltPlace && 
		CurrentPlacementMode != EPlacementMode::MultipleControl && 
		ActivePreviews.Num() <= 0) 
	{
		return false;
	}

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
		OutResultVec = UFactoryGridMathLibrary::CalculateSnappedLocation(HitResult.Location, PlaceObjectPivotGridSize, GridLength);
	}
	
	return bHit;
}

UFactoryPoolSubsystem* UFactoryPlacementComponent::GetPool() const
{
	return GetWorld()->GetGameInstance()->GetSubsystem<UFactoryPoolSubsystem>();
}

AFactoryPlacePreview* UFactoryPlacementComponent::CreateAndInitPreview(
	const UFactoryObjectData* Data, const FVector& Loc, const FRotator& Rot) const
{
	if (!Data) return nullptr;
	
	AFactoryPlacePreview* Preview;
	UFactoryPoolSubsystem* Pool = GetPool();
	if (!Pool) return nullptr;
	
	if (BeltBuilder && Data == BeltBuilder->GetBeltData())
	{
		Preview = Pool->GetItemFromPool<AFactoryBeltPreview>(EFactoryPoolType::BeltPreview, Loc, Rot);
	}
	else if (BeltBuilder && Data == BeltBuilder->GetBeltBridgeData())
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

