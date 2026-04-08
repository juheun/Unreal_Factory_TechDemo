// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FactoryItemRenderSubsystem.generated.h"

class AFactoryBelt;
class AFactoryItemRenderActor;
class UFactoryItemData;
/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryItemRenderSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
	// 외부에서 아이템 랜더 요청
	void RequestRenderItem(const UFactoryItemData* ItemData, const FTransform& Transform);
	void RegisterActiveBelt(AFactoryBelt* Belt);
	void UnregisterActiveBelt(AFactoryBelt* Belt);
	
private:
	UPROPERTY()
	TObjectPtr<AFactoryItemRenderActor> RenderActor;
};
