// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interface/FactoryInteractable.h"
#include "Inventory/FFactorySlot.h"
#include "Logistics/FactoryLogisticsObjectBase.h"
#include "FactoryMachineBase.generated.h"

struct FFactoryItemInstance;
class UFactoryRecipeData;
class UFactoryFacilityItemData;

UCLASS()
class FACTORYTECHDEMO_API AFactoryMachineBase : public AFactoryLogisticsObjectBase, public IFactoryInteractable
{
	GENERATED_BODY()

public:
	AFactoryMachineBase();
	
	virtual void Interact(AActor* Interactor) override;
	virtual FText GetInteractText() const override;
	
	virtual void PlanCycle() override;
	virtual void ExecuteCycle() override;
	virtual void UpdateView() override;
	
	virtual bool CanPushItemFromBeforeObject(const UFactoryInputPortComponent* RequestPort) const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool PullItemFromInputPorts(FFactoryItemInstance& Item) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory|Machine")
	TObjectPtr<UFactoryFacilityItemData> FacilityIdentity;
	
	// 이 설비에서 가공 할 수 있는 레시피. BeginPlay에서 FacilityIdentity를 기반으로 FactoryDataSubsystem에서 가져와서 초기화
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factory|Machine")
	TArray<TObjectPtr<UFactoryRecipeData>> AvailableRecipes;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factory|Machine")
	TObjectPtr<UFactoryRecipeData> CurrentRecipe;
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|Machine")
	TArray<FFactorySlot> InputBufferSlots;
	UPROPERTY(EditDefaultsOnly, Category = "Factory|Machine")
	int32 InputBufferSize = 1;
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|Machine")
	FFactorySlot OutputBufferSlot;
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|Machine")
	int32 RemainingProductionCycle;
	UPROPERTY(VisibleAnywhere, Category = "Factory|Machine")
	bool bIsWorking = false;
	
private:
	void InitMachine();
	bool TryCraftItem();
	bool TryEndCraftItem();
	
	int32 InputPortIndex = 0;
	int32 OutputPortIndex = 0;
};
