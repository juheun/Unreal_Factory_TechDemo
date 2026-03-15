// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryPlayerContextWidget.generated.h"

enum class EPlacementMode : uint8;
enum class EFactoryViewModeType : uint8;
class UTextBlock;

UCLASS()
class FACTORYTECHDEMO_API UFactoryPlayerContextWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateViewModeUI(const FString& ViewModeString) const;
	void UpdatePlacementUI(const FString& PlacementModeString) const;
	void UpdateHotkeyHints(const FString& HotKeyHintString) const;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ViewModeText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PlacementModeText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HotkeyHintText;
};
