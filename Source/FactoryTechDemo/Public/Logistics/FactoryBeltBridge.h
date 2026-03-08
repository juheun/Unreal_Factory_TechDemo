// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryLogisticsObjectBase.h"
#include "FactoryBeltBridge.generated.h"

/**
 * 벨트가 교차하는 블럭에서 직선방향으로 벨트를 이어주는 클래스
 */
UCLASS()
class FACTORYTECHDEMO_API AFactoryBeltBridge : public AFactoryLogisticsObjectBase
{
	GENERATED_BODY()

public:
	AFactoryBeltBridge();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void PlanCycle() override;
	virtual void ExecuteCycle() override;
	virtual void UpdateView() override;
	
	virtual bool CanPushItemFromBeforeObject(const UFactoryInputPortComponent* RequestPort) const override;
	
protected:
	virtual bool PullItemFromInputPorts(FFactoryItemInstance& Item) override;
	
	UPROPERTY()
	TArray<FFactoryItemInstance> CurrentItems;
	
	const int32 LaneCount = 2;
};
