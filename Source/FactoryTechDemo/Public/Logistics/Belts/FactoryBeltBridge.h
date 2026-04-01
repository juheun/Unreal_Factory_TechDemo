// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Logistics/Machines/FactoryLogisticsObjectBase.h"
#include "FactoryBeltBridge.generated.h"

class UFactoryPortComponentBase;

/**
 * 벨트가 교차하는 블럭에서 직선방향으로 벨트를 이어주는 클래스
 */
UCLASS()
class FACTORYTECHDEMO_API AFactoryBeltBridge : public AFactoryLogisticsObjectBase
{
	GENERATED_BODY()

public:
	AFactoryBeltBridge();
	
	virtual void InitPhase() override;
	virtual void LogisticsPhase() override;
	virtual void LateLogisticsPhase() override;
	virtual void LogicPhase() override;
	virtual void VisualPhase() override;
	
	virtual bool CanReceiveItem(UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem) override;
	
	virtual const UFactoryItemData* PeekOutputItem(UFactoryOutputPortComponent* RequestPort) override;
	virtual FFactoryItemInstance ConsumeItem(UFactoryOutputPortComponent* RequestPort) override;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void ReceiveItem(UFactoryInputPortComponent* RequestPort, FFactoryItemInstance Item) override;
	
	UFUNCTION()
	void HandlePortConnectionChanged(UFactoryPortComponentBase* Port, bool bIsConnected);
	
	int32 GetOppositePortIndex(int32 PortIndex) const;
	
private:
	void TryPullInputFromPorts();
	
	UPROPERTY()
	TArray<FFactoryItemInstance> CurrentItems;
	
	UPROPERTY()
	TArray<bool> ReceivedThisCycleFlags;
};
