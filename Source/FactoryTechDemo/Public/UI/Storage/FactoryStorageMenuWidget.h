// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryStorageMenuWidget.generated.h"

class UFactoryWarehousePanelWidget;
class UFactoryInventoryWidget;
class UFactoryInventoryComponent;

UENUM(BlueprintType)
enum class EFactoryMenuMode : uint8
{
	InventoryOnly UMETA(DisplayName = "Inventory Only"),
	Warehouse UMETA(DisplayName = "Warehouse Panel"),
	Facility UMETA(DisplayName = "Facility Panel")
};

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryStorageMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 컨트롤러에서 창고를 열 때 호출해 줄 초기화 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenMenu(UFactoryInventoryComponent* PlayerInventory, EFactoryMenuMode MenuMode);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UFactoryWarehousePanelWidget> WarehousePanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UFactoryInventoryWidget> InventoryPanel;
};
