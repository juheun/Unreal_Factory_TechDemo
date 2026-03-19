// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryItemSelectButton.generated.h"

class UTextBlock;
class UImage;
class UButton;
class UFactoryItemData;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemSelectButtonClicked, UFactoryItemData*, SelectedItem);
UCLASS()
class FACTORYTECHDEMO_API UFactoryItemSelectButton : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	// 버튼 비주얼 초기화
	void InitButton(UFactoryItemData* InItemData);

	UPROPERTY(BlueprintAssignable, Category = "Factory|UI|Event")
	FOnItemSelectButtonClicked OnButtonClicked;
	
protected:
	UFUNCTION()
	void HandleButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ItemButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemNameText;
	
private:
	UPROPERTY()
	TObjectPtr<UFactoryItemData> CachedItemData;
};
