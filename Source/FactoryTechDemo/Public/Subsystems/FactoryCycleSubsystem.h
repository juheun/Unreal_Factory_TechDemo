// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FactoryCycleSubsystem.generated.h"

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
	
	void RegisterLogisticsActor(class AFactoryLogisticsObjectBase* LogisticsObject);
	void UnregisterLogisticsActor(class AFactoryLogisticsObjectBase* LogisticsObject);
	
protected:
	void OnFactoryCycle();
	
private:
	FTimerHandle CycleTimerHandle;
	
	UPROPERTY()
	TArray<class AFactoryLogisticsObjectBase*> RegisteredLogisticsObjectArr;
	
	const float CycleInterval = 1.f;
};
