// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryQuickBarWidget.generated.h"

class UHorizontalBox;
class UFactoryQuickSlotWidget;
class UFactoryQuickSlotComponent;
/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryQuickBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Factory|UI")
	void InitQuickBar(UFactoryQuickSlotComponent* QuickSlotComponent);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
	TSubclassOf<UFactoryQuickSlotWidget> QuickSlotWidgetBP;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> QuickSlotsBox;	// 슬롯들을 가로로 나열할 박스
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UFactoryQuickSlotWidget>> QuickSlotWidgets;
	
	const int32 MaxSlots = 10; // 1부터 0까지 고정. UI는 실 데이터 유무와 상관없이 총 갯수 고정. 
};
