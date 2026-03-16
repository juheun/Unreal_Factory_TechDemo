// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/FactoryInteractionComponent.h"

#include "EnhancedInputComponent.h"
#include "Engine/OverlapResult.h"
#include "UI/Interaction/FactoryInteractionWidget.h"
#include "Interface/FactoryInteractable.h"
#include "Player/FactoryPlayerController.h"
#include "Player/Component/FactoryPlacementComponent.h"
#include "Player/Input/FactoryInputConfig.h"


UFactoryInteractionComponent::UFactoryInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFactoryInteractionComponent::SetUpInputComponent(UEnhancedInputComponent* PlayerInputComp,
	const UFactoryInputConfig* InputConfig)
{
	PlayerInputComp->BindAction(InputConfig->InteractAction, ETriggerEvent::Started, this, &UFactoryInteractionComponent::PerformInteraction);
}

void UFactoryInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CachedPlayerController = Cast<AFactoryPlayerController>(GetOwner());
	
	if (AFactoryPlayerController* Controller = CachedPlayerController.Get())
	{
		if (Controller->IsLocalPlayerController() && InteractionPromptWidgetBP)
		{
			InteractionPromptWidget = CreateWidget<UFactoryInteractionWidget>(Controller, InteractionPromptWidgetBP);
			if (InteractionPromptWidget)
			{
				InteractionPromptWidget->AddToViewport();
				InteractionPromptWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

void UFactoryInteractionComponent::UpdateInteraction() const
{
	// TODO : 여러 상호작용 대상을 찾도록 변경
	AFactoryPlayerController* Controller = CachedPlayerController.Get();
	if (!Controller) return;

	EPlacementMode PlacementMode = Controller->GetCurrentPlacementMode();
	bool bIsInventoryOpen = Controller->GetIsStorageOpen();
	EFactoryViewModeType ViewMode = Controller->GetCurrentViewMode();
	
	bool bHideInteraction = (PlacementMode == EPlacementMode::PlaceFromInventory ||
		PlacementMode == EPlacementMode::BeltPlace || bIsInventoryOpen);
	
	if (!bHideInteraction && InteractionPromptWidget)
	{
		if (TScriptInterface<IFactoryInteractable> BestTarget = FindBestInteractable(ViewMode))
		{
			FText Text;
			if (BestTarget->TryGetInteractText(PlacementMode, Text))
			{
				InteractionPromptWidget->SetInteractionText(Text);
				InteractionPromptWidget->SetVisibility(ESlateVisibility::Visible);
			}
		}
		else
		{
			InteractionPromptWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		InteractionPromptWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UFactoryInteractionComponent::PerformInteraction()
{
	if (AFactoryPlayerController* Controller = CachedPlayerController.Get())
	{
		if (TScriptInterface<IFactoryInteractable> Target = FindBestInteractable(Controller->GetCurrentViewMode()))
		{
			Target->Interact(Controller->GetPawn(), Controller->GetCurrentPlacementMode());
		}
	}
}

TScriptInterface<IFactoryInteractable> UFactoryInteractionComponent::FindBestInteractable(
	const EFactoryViewModeType ViewMode) const
{
	// TODO : 더 좋은 방식으로 변경
	AFactoryPlayerController* Controller = CachedPlayerController.Get();
	if (!Controller) return nullptr;
	
	if (ViewMode == EFactoryViewModeType::NormalView)
	{
		APawn* ControlledPawn = Controller->GetPawn();
		if (!ControlledPawn) return nullptr;

		FVector CharacterLoc = ControlledPawn->GetActorLocation();
    
		TArray<FOverlapResult> Overlaps;
		FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionRange);
    
		// 범위 내 모든 액터 탐색
		GetWorld()->OverlapMultiByChannel(Overlaps, CharacterLoc, FQuat::Identity, ECC_Visibility, Sphere);

		TScriptInterface<IFactoryInteractable> BestTarget = nullptr;
		float ClosestDistanceSqr = TNumericLimits<float>::Max(); // 가장 작은 거리를 찾기 위해 최댓값으로 초기화

		for (auto& Result : Overlaps)
		{
			AActor* OverlapActor = Result.GetActor();
			if (OverlapActor && OverlapActor->Implements<UFactoryInteractable>())
			{
				// 캐릭터와 타겟 사이의 거리 계산 (비교용이므로 루트 연산이 없는 Squared 버전 사용)
				float CurrentDistSqr = FVector::DistSquared(CharacterLoc, OverlapActor->GetActorLocation());

				// 현재까지 찾은 대상 중 가장 가까운지 확인
				if (CurrentDistSqr < ClosestDistanceSqr)
				{
					ClosestDistanceSqr = CurrentDistSqr;
					BestTarget = OverlapActor;
				}
			}
		}
		return BestTarget;
	}
	else
	{	
		// 탑뷰모드
		FHitResult HitResult;
		
		if (Controller->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
		{
			if (AActor* HitActor = HitResult.GetActor())
			{
				if (HitActor->Implements<UFactoryInteractable>())
				{
					return HitActor;
				}
			}
		}
	}

	return nullptr;
}


