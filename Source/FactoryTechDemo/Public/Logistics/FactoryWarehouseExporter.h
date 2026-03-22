// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryLogisticsObjectBase.h"
#include "FactoryWarehouseExporter.generated.h"

class UFactoryItemData;
class UFactorySmartNameplateComponent;
class UFactoryRecipeBillboardComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExporterItemChanged, const UFactoryItemData*, NewItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWarehouseAmountUpdated, int32, CurrentAmount);
UCLASS()
class FACTORYTECHDEMO_API AFactoryWarehouseExporter : public AFactoryLogisticsObjectBase
{
	GENERATED_BODY()

public:
	AFactoryWarehouseExporter();
	
	UPROPERTY(BlueprintAssignable, Category = "Factory|Exporter|Event")
	FOnExporterItemChanged OnTargetItemChanged;
	UPROPERTY(BlueprintAssignable, Category = "Factory|Exporter|Event")
	FOnWarehouseAmountUpdated OnWarehouseAmountUpdated;
	
	virtual void InitObject(const UFactoryObjectData* Data) override;
	
	virtual void PlanCycle() override;
	virtual void ExecuteCycle() override;
	virtual void UpdateView() override;
	
	virtual bool CanPushItemFromBeforeObject(const UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem) const override;
	
	UFUNCTION(BlueprintCallable, Category = "Factory|Exporter")
	void SetTargetItem(const UFactoryItemData* NewTargetItem);
	
	UFUNCTION(BlueprintPure, Category = "Factory|Exporter")
	const UFactoryItemData* GetTargetItem() const { return TargetItemData; }

protected:
	virtual void BeginPlay() override;
	virtual bool PullItemFromInputPorts(FFactoryItemInstance& Item) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const UFactoryItemData> TargetItemData;
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|UI")
	TObjectPtr<UFactorySmartNameplateComponent> SmartNameplateComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|UI")
	TObjectPtr<UFactoryRecipeBillboardComponent> RecipeBillboardComponent;
};
