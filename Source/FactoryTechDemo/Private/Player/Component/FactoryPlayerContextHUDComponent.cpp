// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/FactoryPlayerContextHUDComponent.h"

#include "Blueprint/UserWidget.h"
#include "Player/FactoryPlayerController.h"
#include "UI/PlayerContext/FactoryPlayerContextWidget.h"


UFactoryPlayerContextHUDComponent::UFactoryPlayerContextHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFactoryPlayerContextHUDComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CachedController = Cast<AFactoryPlayerController>(GetOwner());
	
	if (AFactoryPlayerController* Controller = CachedController.Get())
	{
		Controller->OnViewModeChanged.AddDynamic(this, &UFactoryPlayerContextHUDComponent::OnViewModeChanged);
		
		CachedViewMode = Controller->GetCurrentViewMode();
		CachedPlacementMode = Controller->GetCurrentPlacementMode();
		
		if (Controller->IsLocalPlayerController() && ContextWidgetBP)
		{
			ContextWidget = CreateWidget<UFactoryPlayerContextWidget>(Controller, ContextWidgetBP);
			if (ContextWidget)
			{
				ContextWidget->AddToViewport();
				
				// 초기화
				OnViewModeChanged(CachedViewMode);
				OnPlacementModeChanged(CachedPlacementMode);
			}
		}
	}
}

void UFactoryPlayerContextHUDComponent::OnViewModeChanged(EFactoryViewModeType ViewMode)
{
	CachedViewMode = ViewMode;
	
	FString DisplayText;
	switch (CachedViewMode)
	{
	case EFactoryViewModeType::NormalView:
		DisplayText = TEXT("일반 시점");
		break;
	case EFactoryViewModeType::TopView:
		DisplayText = TEXT("탑뷰 시점");
		break;
	}
	ContextWidget->UpdateViewModeUI(DisplayText);
	
	RefreshInputHints();
}

void UFactoryPlayerContextHUDComponent::OnPlacementModeChanged(EPlacementMode PlacementMode)
{
	CachedPlacementMode = PlacementMode;
	
	FString DisplayText;
	switch (CachedPlacementMode)
	{
	case EPlacementMode::PlaceFromInventory:
	case EPlacementMode::Move:
	case EPlacementMode::BeltPlace:
		DisplayText = TEXT("배치모드");
		break;
	case EPlacementMode::Retrieve:
		DisplayText = TEXT("수납모드");
		break;
	case EPlacementMode::None:
	default:
		DisplayText = TEXT("");
	}
	ContextWidget->UpdatePlacementUI(DisplayText);
	
	RefreshInputHints();
}

void UFactoryPlayerContextHUDComponent::RefreshInputHints() const
{
	if (!ContextWidget) return;
	
	// TODO(Optimization): 추후 키 설정 변경 기능 도입 시 개선필요. 현재는 정적 텍스트 사용
	
	FString HintString = TEXT("");
	
	if (CachedPlacementMode == EPlacementMode::None)
	{
		if (CachedViewMode == EFactoryViewModeType::NormalView)
		{
			HintString += TEXT("[WASD] 이동\n");
			HintString += TEXT("[F] 상호작용\n");
			HintString += TEXT("[B] 인벤토리\n");
			HintString += TEXT("[X] 수납모드\n");
			HintString += TEXT("[E] 컨베이어 벨트 배치모드\n");
		}
		else
		{
			HintString += TEXT("[WASD] 카메라 이동\n");
			HintString += TEXT("[마우스 드래그] 카메라 이동\n");
			HintString += TEXT("[Ctrl + R] 카메라 회전\n");
			HintString += TEXT("[마우스 휠] 카메라 줌 인/아웃\n");
			HintString += TEXT("[B] 인벤토리\n");
			HintString += TEXT("[E] 컨베이어 벨트 배치모드\n");
			HintString += TEXT("[마우스 왼쪽 버튼 길게 누르기] 설비 이동\n");
		}
		HintString += TEXT("[CAPSLOCK] 시점 변경\n");
	}
	else if (CachedPlacementMode == EPlacementMode::Retrieve)
	{
		HintString += TEXT("[X] 수납모드 해제\n");
		HintString += TEXT("[F] 설비 수납\n");
	}
	else
	{
		HintString += TEXT("[마우스 왼쪽 버튼] 배치\n");
		HintString += TEXT("[마우스 오른쪽 버튼] 배치 취소\n");
		HintString += TEXT("[R] 회전\n");
		
		if (CachedPlacementMode == EPlacementMode::BeltPlace)
		{
			HintString += TEXT("[E] 컨베이어 벨트 배치모드 해제\n");
		}
	}
	
	ContextWidget->UpdateHotkeyHints(HintString);
}
