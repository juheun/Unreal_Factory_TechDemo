// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryPlaceObjectBase.h"
#include "FactoryLogisticsObjectBase.generated.h"

UCLASS()
class FACTORYTECHDEMO_API AFactoryLogisticsObjectBase : public AFactoryPlaceObjectBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFactoryLogisticsObjectBase();

	virtual void InitObject(const class UFactoryObjectData* Data) override;
	virtual void OnFactoryCycle();	//공장 시스템의 처리단위마다 호출되는 함수
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Logistics")
	TArray<TObjectPtr<class UFactoryLogisticsPortComponent>> LogisticsPortArr;
	
	void InitializeLogisticsPort();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
