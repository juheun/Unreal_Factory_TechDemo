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
	
	virtual void InitPhase() override;
	virtual void LogisticsPhase() override;
	virtual void LateLogisticsPhase() override;
	virtual void LogicPhase() override;
	virtual void VisualPhase() override;
	
	virtual const UFactoryItemData* PeekOutputItem(UFactoryOutputPortComponent* RequestPort) override;
	virtual FFactoryItemInstance ConsumeItem(UFactoryOutputPortComponent* RequestPort) override;
	virtual bool CanReceiveItem(UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem) override;
	
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void ReceiveItem(UFactoryInputPortComponent* RequestPort, FFactoryItemInstance Item) override;
	
private:
	void TryPushOutputToPorts();
	void TryPullInputFromPorts();
	
	UPROPERTY()
	FFactoryItemInstance CurrentItem; 
	
	int32 CurrentInputIndex = 0;
	int32 CurrentOutputIndex = 0;
	
	bool bReceivedThisCycle = false;
};
