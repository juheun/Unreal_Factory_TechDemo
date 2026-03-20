// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryLogisticsObjectBase.h"
#include "FactoryWarehouseImporter.generated.h"

class UFactorySmartNameplateComponent;
class UFactoryRecipeBillboardComponent;

UCLASS()
class FACTORYTECHDEMO_API AFactoryWarehouseImporter : public AFactoryLogisticsObjectBase
{
	GENERATED_BODY()

public:
	AFactoryWarehouseImporter();
	
	virtual void PlanCycle() override;
	virtual void ExecuteCycle() override;
	virtual void UpdateView() override;
	
	virtual bool CanPushItemFromBeforeObject(
		const UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem) const override;
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Factory|UI")
	TObjectPtr<UFactorySmartNameplateComponent> SmartNameplateComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|UI")
	TObjectPtr<UFactoryRecipeBillboardComponent> RecipeBillboardComponent;
};
