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
#include "Player/Component/FactoryFacilitySelectionComponent.h"
#include "Subsystems/FactoryPoolSubsystem.h"


UFactoryPlacementComponent::UFactoryPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	BeltBuilder = CreateDefaultSubobject<UFactoryBeltBuilderComponent>(TEXT("BeltBuilder"));
	SelectionComponent = CreateDefaultSubobject<UFactoryFacilitySelectionComponent>(TEXT("FacilitySelectionComponent"));
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
	PlayerInputComp->BindAction(InputConfig->EnterMoveModeAction, ETriggerEvent::Triggered, this, &UFactoryPlacementComponent::EnterSingleMoveMode);
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
	
	if (SelectionComponent)
	{
		SelectionComponent->OnObjectSelected.AddDynamic(this, &UFactoryPlacementComponent::HandleObjectSelected);
		SelectionComponent->OnObjectDeselected.AddDynamic(this, &UFactoryPlacementComponent::HandleObjectDeselected);
		SelectionComponent->OnSelectionCleared.AddDynamic(this, &UFactoryPlacementComponent::HandleSelectionCleared);
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
				
				bool bShouldChainBuild = true;
				if (AFactoryLogisticsObjectBase* HitFacility = UFactoryGridMathLibrary::GetFacilityAtGrid(this, PointingLoc, GridLength))
				{
					if (!HitFacility->IsA<AFactoryBelt>())
					{
						bShouldChainBuild = false;
					}
				}
				
				if (PlaceObject())
				{
					// 스폰 성공 시, 방금 캐싱한 끝점을 다시 클릭한 것처럼 만들어서 연속 짓기 돌입
					BeltBuilder->ResetBuilderState();
					if (bShouldChainBuild)	// 만약 지금 클릭한 곳이 벨트나 벨트 브릿지가 아니라면, 연속 짓기 모드로 들어가지 않음
					{
						BeltBuilder->ProcessClick(NextStartLoc, GridLength); 
					}
					else
					{
						SetupSinglePreview(BeltBuilder->GetBeltData(), EPlacementMode::BeltPlace);
					}
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
	if (CurrentPlacementMode == EPlacementMode::None)
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
	else if (CurrentPlacementMode == EPlacementMode::BeltPlace)
	{
		CancelPlaceObject();
		CurrentPlacementMode = EPlacementMode::None;
	}

	OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
}

void UFactoryPlacementComponent::ToggleRetrieveMode()
{
	if (CurrentPlacementMode == EPlacementMode::None)
	{
		CancelPlaceObject();	// 기존 배치 모드 취소
		CurrentPlacementMode = EPlacementMode::Retrieve;
	}
	else if (CurrentPlacementMode == EPlacementMode::Retrieve)
	{
		CurrentPlacementMode = EPlacementMode::None;
	}
	
	OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
}

void UFactoryPlacementComponent::EnterSingleMoveMode()
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
				if (HitObject->IsA<AFactoryBelt>())
				{
					return;		// 기획상 벨트는 이동할 수 없게 방어
				}
				if (SelectionComponent) SelectionComponent->SelectObject(HitObject);
				
				HitObject->SetActorEnableCollision(false);
				
				CurrentPlacementMode = EPlacementMode::Move;
				OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
				StartObjectPlaceMode();
			}
		}
	}
}

void UFactoryPlacementComponent::ToggleMultipleControlMode()
{
	if (CurrentPlacementMode == EPlacementMode::None)
	{
		CurrentPlacementMode = EPlacementMode::MultipleControl;
		if (SelectionComponent) SelectionComponent->ClearSelectedObjects();
		OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
	}
	else if (CurrentPlacementMode == EPlacementMode::MultipleControl)
	{
		CancelPlaceObject();
	}
}

#pragma endregion

#pragma region 배치 및 프리뷰 제어

/**
 * 객체 데이터 하나를 받아서 해당 객체 하나에 대한 프리뷰를 띄우는 함수. 기존 프리뷰는 모두 제거하고 새로 시작함
 * @param Data 프리뷰를 띄울 ObjectData
 * @param Mode 프리뷰를 띄우며 진입할 배치모드
 */
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

void UFactoryPlacementComponent::ClearAllPreviews()
{
	UFactoryPoolSubsystem* Pool = GetPool();
	for (auto Preview : ActivePreviews)
	{
		if (AFactoryLogisticsObjectBase* OriginalObject = Preview->GetOriginalObject())
		{
			OriginalObject->SetActorHiddenInGame(false);
			OriginalObject->SetActorEnableCollision(true);
		}
		
		if (Preview)
		{
			if (Pool) Pool->ReturnItemToPool(Preview);
			else Preview->Destroy();
		}
	}
	ActivePreviews.Empty();
	
	if (PlaceObjectPivotActor)
	{
		PlaceObjectPivotActor->SetActorHiddenInGame(true);
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
		SetAccuratePlacementPivotCenterAndGridSize();
		
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
	
	SetAccuratePlacementPivotCenterAndGridSize();
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
	
	if (CurrentPlacementMode == EPlacementMode::Move && SelectionComponent)
	{
		TArray<TObjectPtr<AFactoryLogisticsObjectBase>> ObjsToDestroy = SelectionComponent->GetSelectedObjects();
		SelectionComponent->ClearSelectedObjects();
		for (auto& Obj : ObjsToDestroy)
		{
			if (IsValid(Obj)) Obj->Destroy();
		}
	}
	
	ClearAllPreviews();
	
	// 벨트, 다중 제어모드는 배치 후에도 계속 해당모드로 남아있음. 나머지 모드는 배치 후 None으로 돌아감
	if (CurrentPlacementMode != EPlacementMode::BeltPlace && CurrentPlacementMode != EPlacementMode::MultipleControl)
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
		if (SelectionComponent) SelectionComponent->ClearSelectedObjects();
	}
	
	if (BeltBuilder) BeltBuilder->ResetBuilderState();
	CurrentPlacementMode = EPlacementMode::None;
	ClearAllPreviews();
	OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
}

/**
 * 현재 존재하는 Preview객체의 위치에 따라 Pivot 객체의 GridSize와 위치를 지정함
 */
void UFactoryPlacementComponent::SetAccuratePlacementPivotCenterAndGridSize()
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
	else if (CurrentPlacementMode == EPlacementMode::Move && SelectionComponent->GetSelectedObjects().Num() > 0)
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
	
	// 1차적으로 이미 존재하는 벨트를 풀에 집어넣음
	for (auto& Preview : ActivePreviews)
	{
		if (Preview) Pool->ReturnItemToPool(Preview);
	}
	ActivePreviews.Empty();
	
	// BeltPlacementDatas의 데이터에 따라 새로운 벨트 프리뷰 생성 (Pool에서 가져옴)
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

#pragma region 객체 선택 관련

void UFactoryPlacementComponent::OnMultipleControlAddStarted()
{
	if (CurrentPlacementMode != EPlacementMode::MultipleControl) return;
	
	FVector HitLocation;
	if (TryGetPointingGridLocation(HitLocation) && SelectionComponent)
	{
		SelectionComponent->BeginDragSelection(HitLocation, GridLength, false);
	}
}

void UFactoryPlacementComponent::OnMultipleControlAddCompleted()
{
	if (CurrentPlacementMode != EPlacementMode::MultipleControl) return;
	
	FVector HitLocation;
	if (TryGetPointingGridLocation(HitLocation) && SelectionComponent)
	{
		if (SelectionComponent->EndDragSelection(HitLocation, GridLength))
		{
			SelectionComponent->SelectConnectedBeltLine(HitLocation, GridLength);
		}
	}
}

void UFactoryPlacementComponent::OnMultipleControlRemoveStarted()
{
	if (CurrentPlacementMode != EPlacementMode::MultipleControl) return;
	
	FVector HitLocation;
	if (TryGetPointingGridLocation(HitLocation) && SelectionComponent)
	{
		SelectionComponent->BeginDragSelection(HitLocation, GridLength, true);
	}
}

void UFactoryPlacementComponent::OnMultipleControlRemoveCompleted()
{
	if (CurrentPlacementMode != EPlacementMode::MultipleControl) return;
	
	FVector HitLocation;
	if (TryGetPointingGridLocation(HitLocation) && SelectionComponent)
	{
		// 일반 클릭이면 모드 종료
		if (SelectionComponent->EndDragSelection(HitLocation, GridLength))
		{
			ToggleMultipleControlMode();
		}
	}
}

void UFactoryPlacementComponent::UpdateDragSelectionBox()
{
	if (CurrentPlacementMode != EPlacementMode::MultipleControl) return;
	
	FVector HitLocation;
	if (TryGetPointingGridLocation(HitLocation) && SelectionComponent)
	{
		SelectionComponent->UpdateDragSelectionBox(HitLocation, GridLength);
	}
}

void UFactoryPlacementComponent::OnMultipleControlMove()
{
	if (CurrentPlacementMode == EPlacementMode::MultipleControl && SelectionComponent->GetSelectedObjects().Num() > 0)
	{
		for (auto& Obj : SelectionComponent->GetSelectedObjects())
		{
			if (Obj) Obj->SetActorEnableCollision(false);
		}
		
		CurrentPlacementMode = EPlacementMode::Move;
		OnPlacementModeChanged.Broadcast(CurrentPlacementMode);
		StartObjectPlaceMode();
	}
}

void UFactoryPlacementComponent::OnMultipleControlRetrieve()
{
	if (CurrentPlacementMode == EPlacementMode::MultipleControl && SelectionComponent && SelectionComponent->GetSelectedObjects().Num() > 0)
	{
		// 데이터 복사 후 철거 (Clear하면 배열이 날아가므로)
		TArray<TObjectPtr<AFactoryLogisticsObjectBase>> ObjectsToRetrieve = SelectionComponent->GetSelectedObjects();
		SelectionComponent->ClearSelectedObjects(); 
		
		for (auto Obj : ObjectsToRetrieve)
		{
			if (Obj) Obj->Retrieve();
		}
	}
}

void UFactoryPlacementComponent::HandleObjectSelected(AFactoryLogisticsObjectBase* SelectedObject)
{
	if (!SelectedObject) return;

	// 원본 숨기기
	SelectedObject->SetActorHiddenInGame(true);

	// 프리뷰 생성해서 ActivePreviews에 넣기
	AFactoryPlacePreview* Preview = CreateAndInitPreview(SelectedObject->GetObjectData(), 
		SelectedObject->GetActorLocation(), SelectedObject->GetActorRotation(), SelectedObject);
	if (Preview)
	{
		if (Preview->IsA(AFactoryBeltPreview::StaticClass()))
		{
			Cast<AFactoryBeltPreview>(Preview)->SetBeltType(Cast<AFactoryBelt>(SelectedObject)->GetBeltType());
		}
		ActivePreviews.Add(Preview);
	}
}

void UFactoryPlacementComponent::HandleObjectDeselected(AFactoryLogisticsObjectBase* DeselectedObject)
{
	if (!IsValid(DeselectedObject)) return; // 이동/철거 등으로 이미 삭제된 객체라면 무시

	// 원본 다시 보이기
	DeselectedObject->SetActorHiddenInGame(false);
	DeselectedObject->SetActorEnableCollision(true);
	
	for (int i = 0; i < ActivePreviews.Num(); i++)
	{
		AFactoryPlacePreview* Preview = ActivePreviews[i];
		if (Preview && Preview->GetOriginalObject() == DeselectedObject)
		{
			UFactoryPoolSubsystem* Pool = GetPool();
			if (Pool) Pool->ReturnItemToPool(Preview);
			else Preview->Destroy();
          
			ActivePreviews.RemoveAt(i);
			break;
		}
	}
}

void UFactoryPlacementComponent::HandleSelectionCleared()
{
	ClearAllPreviews();
}

#pragma endregion

#pragma region 유틸리티

bool UFactoryPlacementComponent::TryGetPointingGridLocation(FVector& OutResultVec) const
{
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
	const UFactoryObjectData* Data, const FVector& Loc, const FRotator& Rot, AFactoryLogisticsObjectBase* OriginalObject) const
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
		Preview->InitPreview(Data, OriginalObject);
		Preview->SetActorEnableCollision(true);
	}
	
	return Preview;
}

#pragma endregion

