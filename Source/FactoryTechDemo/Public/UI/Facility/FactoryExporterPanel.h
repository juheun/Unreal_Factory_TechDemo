// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryFacilityPanelBase.h"
#include "FactoryExporterPanel.generated.h"

class UTextBlock;
class AFactoryWarehouseExporter;
class UFactoryItemSelectionPopup;
class UFactoryBaseSlotWidget;
class UButton;
class UFactoryItemData;
/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryExporterPanel : public UFactoryFacilityPanelBase
{
	GENERATED_BODY()
	
public:
	virtual void InitPanel(AFactoryPlaceObjectBase* PlaceObject) override;
	
protected:
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void OnSelectPanelOpenButtonClicked();	// 아이템 선택 패널 열기 버튼
	
	UFUNCTION()
	void OnPopupItemSelected(const UFactoryItemData* SelectedItem);	// 팝업에서 아이템 선택 후 닫기
	
	UFUNCTION()
	void OnExporterItemChanged(const UFactoryItemData* NewItem); // Exporter의 타겟 아이템 바꿨을 때 슬롯 UI 갱신하는 함수
	
	UFUNCTION()
	void OnWarehouseItemAmountUpdated(const int32 CurrentAmount);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SelectPanelOpenButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UFactoryBaseSlotWidget> TargetItemSlot;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WarehouseAmountText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UFactoryItemSelectionPopup> ItemSelectionPopup;
	
private:
	UPROPERTY()
	TWeakObjectPtr<AFactoryWarehouseExporter> CachedExporter;
};
