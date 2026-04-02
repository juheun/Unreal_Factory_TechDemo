// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/FactoryFacilitySelectionComponent.h"

#include "Libraries/FactoryGridMathLibrary.h"
#include "Logistics/Belts/FactoryBelt.h"
#include "Player/Input/FactoryPlayerController.h"
#include "UI/PlayerContext/FactoryDragSelectionWidget.h"


UFactoryFacilitySelectionComponent::UFactoryFacilitySelectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFactoryFacilitySelectionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CachedPlayerController = Cast<AFactoryPlayerController>(GetOwner());
	InitializeSelectionUI();
}

void UFactoryFacilitySelectionComponent::InitializeSelectionUI()
{
	if (CachedPlayerController.IsValid() && DragSelectionWidgetBP)
	{
		DragSelectionWidget = CreateWidget<UFactoryDragSelectionWidget>(CachedPlayerController.Get(), DragSelectionWidgetBP);
		if (DragSelectionWidget)
		{
			DragSelectionWidget->AddToViewport();
			DragSelectionWidget->StopDrag(); // 처음에 위젯 숨기기
		}
	}
}

void UFactoryFacilitySelectionComponent::BeginDragSelection(const FVector& HitLocation, float GridLength,
	bool bIsRemove)
{
	SelectionDragStartPoint = UFactoryGridMathLibrary::WorldToGrid(HitLocation, GridLength);
	bIsDraggingSelection = true;
	bIsRemoveDrag = bIsRemove;
	
	// 드래그 시작 전 선택 상황을 기억해놓음
	PreDragSelection.Empty();
	PreDragSelection.Append(SelectedObjects);
		
	if (DragSelectionWidget)
	{
		DragSelectionWidget->StartDrag(HitLocation);
	}
}

void UFactoryFacilitySelectionComponent::UpdateDragSelectionBox(const FVector& HitLocation,
	float GridLength)
{
	if (!bIsDraggingSelection) return;
	
	// 드래그 박스 안 겹치는 객체 도출
	FIntPoint CurrentGrid = UFactoryGridMathLibrary::WorldToGrid(HitLocation, GridLength);
	TArray<AFactoryLogisticsObjectBase*> OverlappedObjects = 
		UFactoryGridMathLibrary::GetFacilitiesInGridBox(this, SelectionDragStartPoint, CurrentGrid, GridLength);
	
	// 목표 상태 계산
	TSet<TObjectPtr<AFactoryLogisticsObjectBase>> DesiredSelection = PreDragSelection;
	for (auto* HitObj : OverlappedObjects)
	{
		if (bIsRemoveDrag) DesiredSelection.Remove(HitObj);
		else DesiredSelection.Add(HitObj);
	}
	// 없애야할 객체 Deselect
	for (int i = SelectedObjects.Num() - 1; i >= 0; i--)
	{
		if (!DesiredSelection.Contains(SelectedObjects[i]))
		{
			DeselectObject(SelectedObjects[i]);
		}
	}
	// 추가해야 할 객체 Select
	for (auto Obj : DesiredSelection)
	{
		SelectObject(Obj);
	}
	
	// 드래그 선택 위젯 업데이트
	if (AFactoryPlayerController* PlayerController = CachedPlayerController.Get())
	{
		if (DragSelectionWidget)
		{
			float MouseX, MouseY;
			if (PlayerController->GetMousePosition(MouseX, MouseY))
			{
				DragSelectionWidget->UpdateDrag(PlayerController, FVector2D(MouseX, MouseY));
			}
		}
	}
}

/**
 * 드래그 선택 종료. 반환값이 true면 단순 클릭으로 판정됨을 알림
 * @param HitLocation 마우스 위치
 * @param GridLength 그리드 길이
 * @return 단순 클릭 여부 (False면 Drag 종료)
 */
bool UFactoryFacilitySelectionComponent::EndDragSelection(const FVector& HitLocation, float GridLength)
{
	if (!bIsDraggingSelection) return false;
	bIsDraggingSelection = false;
	
	if (DragSelectionWidget)
	{
		DragSelectionWidget->StopDrag();
	}

	// StartGrid와 EndGrid가 같다면 드래그가 아닌 단순 클릭으로 판정
	if (SelectionDragStartPoint == UFactoryGridMathLibrary::WorldToGrid(HitLocation, GridLength))
	{
		return true; // 메인 컴포넌트에게 단순 클릭임을 알림
	}
	
	return false;
}

void UFactoryFacilitySelectionComponent::SelectObject(AFactoryLogisticsObjectBase* TargetObject)
{
	if (!TargetObject || SelectedObjects.Contains(TargetObject)) return;
	SelectedObjects.Add(TargetObject);
	
	OnObjectSelected.Broadcast(TargetObject);
}

void UFactoryFacilitySelectionComponent::DeselectObject(AFactoryLogisticsObjectBase* TargetObject)
{
	if (!TargetObject || !SelectedObjects.Contains(TargetObject)) return;
	SelectedObjects.Remove(TargetObject);
	
	OnObjectDeselected.Broadcast(TargetObject);
}

void UFactoryFacilitySelectionComponent::ClearSelectedObjects()
{
	TArray<TObjectPtr<AFactoryLogisticsObjectBase>> TempObjects = SelectedObjects;
	SelectedObjects.Empty();
	PreDragSelection.Empty();
	
	OnSelectionCleared.Broadcast();
}

void UFactoryFacilitySelectionComponent::SelectConnectedBeltLine(const FVector& HitLocation, float GridLength)
{
	if (AActor* HitActor = UFactoryGridMathLibrary::GetFacilityAtGrid(this, HitLocation, GridLength))
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

