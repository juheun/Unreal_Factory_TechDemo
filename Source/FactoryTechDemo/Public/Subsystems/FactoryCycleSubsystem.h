// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FactoryCycleSubsystem.generated.h"

class AFactoryLogisticsObjectBase;

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryCycleSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void RegisterLogisticsActor(AFactoryLogisticsObjectBase* LogisticsObject);
	void UnregisterLogisticsActor(AFactoryLogisticsObjectBase* LogisticsObject);
	
	float GetCycleAlpha() const;
	float GetCycleInterval() const { return CycleInterval; }
	
protected:
	void OnFactoryCycle();
	// RegisteredLogisticsObjectArr의 위상정렬 구현
	void SortRegisteredLogisticsObjectArr();	
	
private:
	FTimerHandle CycleTimerHandle;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<AFactoryLogisticsObjectBase>> RegisteredLogisticsObjectArr;
	
	float LastStartCycleTime = 0.f;
	bool bGraphDirty = false;	// 사이클 내부에서 설비의 위상정렬의 재설정을 하는 플래그
	
	const float CycleInterval = 2.f;
};
