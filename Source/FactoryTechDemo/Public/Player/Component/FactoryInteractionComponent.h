// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactoryInteractionComponent.generated.h"


struct FInputActionValue;
struct FInteractionOption;
class UFactoryInputConfig;
class AFactoryPlayerController;
class UFactoryInteractionWidget;
class IFactoryInteractable;
enum class EFactoryViewModeType : uint8;
enum class EPlacementMode : uint8;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFactoryInteractionComponent();
	void SetUpInputComponent(UEnhancedInputComponent* PlayerInputComp, const UFactoryInputConfig* InputConfig);
	
	virtual void BeginPlay() override;
	
	// Controller의 업데이트마다 호출
	void UpdateInteractionTextList();

protected:
	// 상호작용 실행
	void PerformInteraction();
	
	void ScrollInteractionOptions(const FInputActionValue& Value);
	
	UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
	TSubclassOf<UFactoryInteractionWidget> InteractionPromptWidgetBP;
	
	int32 CurrentSelectedIndex = 0;             // 현재 선택된 리스트 인덱스
	TArray<FInteractionOption> CurrentOptions;  // 타겟이 제공하는 옵션 목록
	
private:
	// 상호작용하기에 가장 적합한 대상 탐색
	TScriptInterface<IFactoryInteractable> FindBestInteractable(const EFactoryViewModeType ViewMode) const;
	void ResetInteractionTextList();
	
	UFUNCTION()
	void OnViewModeChangedCallback(EFactoryViewModeType NewViewMode);
	UFUNCTION()
	void OnPlacementModeChangedCallback(EPlacementMode NewPlacementMode);
	
	UPROPERTY()
	TScriptInterface<IFactoryInteractable> CurrentInteractTarget;
	
	UPROPERTY()
	TObjectPtr<UFactoryInteractionWidget> InteractionPromptWidget;
	
	UPROPERTY(EditDefaultsOnly, Category = "Factory|Interaction")
	float InteractionRange = 100.f;
	
	UPROPERTY()
	TWeakObjectPtr<AFactoryPlayerController> CachedPlayerController;
};
