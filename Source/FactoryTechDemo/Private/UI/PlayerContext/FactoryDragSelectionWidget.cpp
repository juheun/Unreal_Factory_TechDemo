// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerContext/FactoryDragSelectionWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"

void UFactoryDragSelectionWidget::StartDrag(const FVector& WorldLocation)
{
	WorldStartLocation = WorldLocation;
	bIsDragging = true;
	if (SelectionBorder)
	{
		SelectionBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UFactoryDragSelectionWidget::UpdateDrag(APlayerController* PC, const FVector2D& CurrentMousePos)
{
	if (!bIsDragging || !PC || !SelectionBorder) return;
	
	FVector2D ScreenStartPos;
	// 저장해둔 3D 월드 좌표를 매 프레임 현재 카메라 기준 2D 화면 좌표로 변환
	if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldStartLocation, ScreenStartPos, false))
	{
		// 뷰포트 스케일을 고려한 보정
		FVector2D WidgetMousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);

		float MinX = FMath::Min(ScreenStartPos.X, WidgetMousePos.X);
		float MinY = FMath::Min(ScreenStartPos.Y, WidgetMousePos.Y);
		float SizeX = FMath::Abs(ScreenStartPos.X - WidgetMousePos.X);
		float SizeY = FMath::Abs(ScreenStartPos.Y - WidgetMousePos.Y);

		// Border의 위치와 크기 실시간 업데이트
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(SelectionBorder->Slot))
		{
			CanvasSlot->SetPosition(FVector2D(MinX, MinY));
			CanvasSlot->SetSize(FVector2D(SizeX, SizeY));
		}
	}
}

void UFactoryDragSelectionWidget::StopDrag()
{
	bIsDragging = false;
	if (SelectionBorder)
	{
		SelectionBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}
