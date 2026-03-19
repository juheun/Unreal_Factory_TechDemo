// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryStorageMenuWidget.generated.h"

class AFactoryPlaceObjectBase;
class UFactoryWarehousePanelWidget;
class UFactoryInventoryWidget;
class UFactoryInventoryComponent;
class UFactoryFacilityPanelBase;
class UBorder;

UENUM(BlueprintType)
enum class EFactoryMenuMode : uint8
{
	InventoryOnly UMETA(DisplayName = "Inventory Only"),
	Warehouse UMETA(DisplayName = "Warehouse Panel"),
	Facility UMETA(DisplayName = "Facility Panel"),
	FacilityOnly UMETA(DisplayName = "Facility Only Panel")
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
	void OpenMenu(UFactoryInventoryComponent* PlayerInventory, EFactoryMenuMode MenuMode, 
		AFactoryPlaceObjectBase* TargetFacility = nullptr);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UFactoryWarehousePanelWidget> WarehousePanel;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> FacilityPanelContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UFactoryInventoryWidget> InventoryPanel;
	
private:
	UPROPERTY()
	TMap<TSubclassOf<UFactoryFacilityPanelBase>, UFactoryFacilityPanelBase*> CachedFacilityPanels;
};
