// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryItemSelectionPopup.generated.h"

class UFactoryItemData;
class UButton;
class UWrapBox;
class UFactoryItemSelectButton;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPopupItemSelected, UFactoryItemData*, SelectedItem);
UCLASS()
class FACTORYTECHDEMO_API UFactoryItemSelectionPopup : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Factory|UI")
	void OpenPopup(const TArray<UFactoryItemData*>& ItemDatas);

	// 팝업 닫기 기능
	UFUNCTION(BlueprintCallable, Category = "Factory|UI")
	void ClosePopup();

	UPROPERTY(BlueprintAssignable, Category = "Factory|UI|Event")
	FOnPopupItemSelected OnItemSelected;
	
protected:
	UFUNCTION()
	void HandleItemButtonClicked(UFactoryItemData* SelectedItem);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> ItemContainer;

	// 우상단 X 닫기 버튼
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	// 동적 생성할 버튼 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
	TSubclassOf<UFactoryItemSelectButton> ItemButtonBP;
};
