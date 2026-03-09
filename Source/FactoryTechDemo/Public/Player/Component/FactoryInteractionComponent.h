// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactoryInteractionComponent.generated.h"


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
	void UpdateInteraction() const;

protected:
	// 상호작용 실행
	void PerformInteraction();
	
	UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
	TSubclassOf<UFactoryInteractionWidget> InteractionPromptWidgetBP;
	
private:
	// 상호작용하기에 가장 적합한 대상 탐색
	TScriptInterface<IFactoryInteractable> FindBestInteractable(const EFactoryViewModeType ViewMode) const;
	
	UPROPERTY()
	TObjectPtr<UFactoryInteractionWidget> InteractionPromptWidget;
	
	UPROPERTY(EditDefaultsOnly, Category = "Factory|Interaction")
	float InteractionRange = 100.f;
	
	UPROPERTY()
	TWeakObjectPtr<AFactoryPlayerController> CachedPlayerController;
};
