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

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Logistics")
	TArray<TObjectPtr<class UFactoryLogisticsPortComponent>> LogisticsPortArr;
	
	void InitializeLogisticsPort();
};
