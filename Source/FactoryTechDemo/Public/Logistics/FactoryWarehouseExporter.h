// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryLogisticsObjectBase.h"
#include "FactoryWarehouseExporter.generated.h"

class UFactoryItemData;

UCLASS()
class FACTORYTECHDEMO_API AFactoryWarehouseExporter : public AFactoryLogisticsObjectBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFactoryWarehouseExporter();
	
	virtual void PlanCycle() override;
	virtual void ExecuteCycle() override;
	virtual void UpdateView() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UFactoryItemData> ItemData;
};
