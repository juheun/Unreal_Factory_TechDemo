// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryFacilityPanelBase.generated.h"

class AFactoryPlaceObjectBase;
/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryFacilityPanelBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void InitPanel(AFactoryPlaceObjectBase* PlaceObject);	// 대응되는 설비를 메서드 내부에서 캐스팅해서 바인딩
	
protected:
	virtual void NativeDestruct() override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Factory")
	TWeakObjectPtr<AFactoryPlaceObjectBase> TargetFacility;
};
