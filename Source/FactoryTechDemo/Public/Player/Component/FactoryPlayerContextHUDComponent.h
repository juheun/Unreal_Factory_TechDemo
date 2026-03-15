// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactoryPlayerContextHUDComponent.generated.h"


class AFactoryPlayerController;
class UInputAction;
enum class EFactoryViewModeType : uint8;
enum class EPlacementMode : uint8;
class UFactoryPlayerContextWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryPlayerContextHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFactoryPlayerContextHUDComponent();

	UFUNCTION()
	void OnViewModeChanged(EFactoryViewModeType ViewMode);
	UFUNCTION()
	void OnPlacementModeChanged(EPlacementMode PlacementMode);

protected:
	virtual void BeginPlay() override;
	void RefreshInputHints() const;
	
	UPROPERTY(EditDefaultsOnly, Category="Factory|UI")
	TSubclassOf<UFactoryPlayerContextWidget> ContextWidgetBP;
	
private:
	UPROPERTY()
	TObjectPtr<UFactoryPlayerContextWidget> ContextWidget;
	
	UPROPERTY()
	TWeakObjectPtr<AFactoryPlayerController> CachedController;
	
	EFactoryViewModeType CachedViewMode;
	EPlacementMode CachedPlacementMode;
};
