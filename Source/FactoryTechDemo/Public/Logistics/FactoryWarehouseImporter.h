// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryLogisticsObjectBase.h"
#include "FactoryWarehouseImporter.generated.h"

class UFactorySmartNameplateComponent;
class UFactoryRecipeBillboardComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnImportItemChanged, const UFactoryItemData*, IncomingItem);
UCLASS()
class FACTORYTECHDEMO_API AFactoryWarehouseImporter : public AFactoryLogisticsObjectBase
{
	GENERATED_BODY()

public:
	AFactoryWarehouseImporter();
	
	UPROPERTY(BlueprintAssignable, Category = "Factory|Importer|Event")
	FOnImportItemChanged OnImportItemChanged;
	
	virtual void PlanCycle() override;
	virtual void LatePlanCycle() override;
	virtual void ExecuteCycle() override;
	virtual void UpdateView() override;
	
	virtual void InitObject(const UFactoryObjectData* Data) override;
	
	virtual bool CanReceiveItem(UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem) override;
	
protected:
	virtual void BeginPlay() override;
	
	virtual void ReceiveItem(UFactoryInputPortComponent* RequestPort, FFactoryItemInstance Item) override;
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|UI")
	TObjectPtr<UFactorySmartNameplateComponent> SmartNameplateComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|UI")
	TObjectPtr<UFactoryRecipeBillboardComponent> RecipeBillboardComponent;
	
private:
	void TryPullInputFromPorts();
	void ProcessImport(FFactoryItemInstance Item);
	
	UPROPERTY()
	TObjectPtr<const UFactoryItemData> CachedLastImportedItem;
	
	// 인풋 포트 막힘 상태 기록용
	bool bIsInputBlocked = false;
};
