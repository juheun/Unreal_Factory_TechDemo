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
	if (InputConfig)
	{
		PlayerInputComp->BindAction(InputConfig->InteractAction, ETriggerEvent::Triggered, this, &UFactoryInteractionComponent::PerformInteraction);
		PlayerInputComp->BindAction(InputConfig->InteractScrollAction, ETriggerEvent::Triggered, this, &UFactoryInteractionComponent::ScrollInteractionOptions);
	}
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
				InteractionPromptWidget->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		
		Controller->OnViewModeChanged.AddDynamic(this, &UFactoryInteractionComponent::OnViewModeChangedCallback);
		
		if (UFactoryPlacementComponent* PlacementComp = Controller->GetComponentByClass<UFactoryPlacementComponent>())
		{
			PlacementComp->OnPlacementModeChanged.AddDynamic(this, &UFactoryInteractionComponent::OnPlacementModeChangedCallback);
		}
	}
}

void UFactoryInteractionComponent::UpdateInteractionTextList()
{
	// TODO : 필요시 여러 상호작용 대상을 찾도록 변경
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
			if (BestTarget != CurrentInteractTarget)
			{
				CurrentInteractTarget = BestTarget;
				CurrentSelectedIndex = 0;
				CurrentOptions.Empty();
				if (BestTarget->TryGetInteractionOptions(PlacementMode, CurrentOptions))
				{
					InteractionPromptWidget->SetInteractionTextList(CurrentOptions, CurrentSelectedIndex);
				}
			}
			bool bIsOptionExist = CurrentOptions.Num() > 0;
			InteractionPromptWidget->SetVisibility(bIsOptionExist ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
			return;
		}
	}
	
	// 만약 타겟이 없거나 숨겨야 할 때 초기화
	ResetInteractionTextList();
}

void UFactoryInteractionComponent::PerformInteraction()
{
	if (AFactoryPlayerController* Controller = CachedPlayerController.Get())
	{
		if (CurrentInteractTarget && CurrentOptions.IsValidIndex(CurrentSelectedIndex))
		{
			CurrentInteractTarget->Interact(
				Controller->GetPawn(), Controller->GetCurrentPlacementMode(), CurrentSelectedIndex);
		}
	}
}

void UFactoryInteractionComponent::ScrollInteractionOptions(const FInputActionValue& Value)
{
	if (CurrentOptions.Num() <= 1) return; // 옵션이 2개 이상이 아니라 실행 될 필요가 없는 경우
	
	float ScrollValue = Value.Get<float>();
	if (ScrollValue > 0.f) // 휠 업
	{
		CurrentSelectedIndex--;
	}
	else if (ScrollValue < 0.f)	// 휠 다운
	{
		CurrentSelectedIndex++;
	}
	CurrentSelectedIndex = FMath::Clamp(CurrentSelectedIndex, 0, CurrentOptions.Num() - 1);
	
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->SetInteractionTextList(CurrentOptions, CurrentSelectedIndex);
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

void UFactoryInteractionComponent::ResetInteractionTextList()
{
	CurrentInteractTarget = nullptr;
	CurrentOptions.Empty();
	CurrentSelectedIndex = 0;
	
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFactoryInteractionComponent::OnViewModeChangedCallback(EFactoryViewModeType NewViewMode)
{
	ResetInteractionTextList();
}

void UFactoryInteractionComponent::OnPlacementModeChangedCallback(EPlacementMode NewPlacementMode)
{
	ResetInteractionTextList();
}


