// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryItemRenderActor.generated.h"

class UInstancedStaticMeshComponent;
class UFactoryItemData;
class AFactoryBelt;

UCLASS()
class FACTORYTECHDEMO_API AFactoryItemRenderActor : public AActor
{
	GENERATED_BODY()

public:
	AFactoryItemRenderActor();
	
	virtual void Tick(float DeltaTime) override;
	
	void InitializeRenderers();
	void RequestRenderItem(const UFactoryItemData* ItemData, const FTransform& Transform);
	
	void RegisterActiveBelt(AFactoryBelt* Belt);
	void UnRegisterActiveBelt(AFactoryBelt* Belt);
	
private:
	UPROPERTY()
	TMap<const UFactoryItemData*, UInstancedStaticMeshComponent*> ItemISMMap;
	TMap<const UFactoryItemData*, TArray<FTransform>> RenderRequests;	// 이번 프레임에 그려야하는 아이템 위치 배열
	UPROPERTY()
	TArray<TWeakObjectPtr<AFactoryBelt>> ActiveBelts;
};
