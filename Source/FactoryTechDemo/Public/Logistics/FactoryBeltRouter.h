// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryLogisticsObjectBase.h"
#include "FactoryBeltRouter.generated.h"

/**
 * 분배기 및 합류기를 위한 라운드 로빈 로직 구현 클래스
 */
UCLASS()
class FACTORYTECHDEMO_API AFactoryBeltRouter : public AFactoryLogisticsObjectBase
{
	GENERATED_BODY()

public:
	AFactoryBeltRouter();
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void PlanCycle() override;
	virtual void ExecuteCycle() override;
	virtual void UpdateView() override;
	
	virtual bool CanPushItemFromBeforeObject(
		UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem) override;
	
protected:
	virtual bool PullItemFromInputPorts(FFactoryItemInstance& Item) override;
	
	UPROPERTY()
	FFactoryItemInstance CurrentItem; 
	
	int32 CurrentInputIndex = 0;
	int32 CurrentOutputIndex = 0;
};
