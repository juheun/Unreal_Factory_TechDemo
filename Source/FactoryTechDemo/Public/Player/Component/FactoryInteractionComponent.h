// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactoryInteractionComponent.generated.h"


class AFactoryPlayerController;
class UFactoryInteractionWidget;
class IFactoryInteractable;
enum class EFactoryViewModeType : uint8;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFactoryInteractionComponent();
	
	virtual void BeginPlay() override;
	
	// Controller의 업데이트마다 호출
	void UpdateInteraction(const EFactoryViewModeType ViewMode, bool bIsPlaceMode, bool bIsInventoryOpen);
	// 상호작용 실행
	void PerformInteraction(APawn* Interacter, const EFactoryViewModeType ViewMode);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
	TSubclassOf<UFactoryInteractionWidget> InteractionPromptWidgetBP;
	
private:
	// 상호작용하기에 가장 적합한 대상 탐색
	TScriptInterface<IFactoryInteractable> FindBestInteractable(const EFactoryViewModeType ViewMode);
	
	UPROPERTY()
	TObjectPtr<UFactoryInteractionWidget> InteractionPromptWidget;
	
	float InteractionRange = 300.f;
	
	UPROPERTY()
	TWeakObjectPtr<AFactoryPlayerController> CachedPlayerController;
};
