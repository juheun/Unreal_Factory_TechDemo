// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryItemVisual.generated.h"

UCLASS()
class FACTORYTECHDEMO_API AFactoryItemVisual : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFactoryItemVisual();
	
	void UpdateVisual(const class UFactoryItemData* ItemData);	// 아이템의 외관 업데이트
	
	UStaticMeshComponent* GetMeshComponent() const {return MeshComponent;}

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;	// 런타임중 메쉬를 바꾸기 위한 다이나믹 마테리얼
};
