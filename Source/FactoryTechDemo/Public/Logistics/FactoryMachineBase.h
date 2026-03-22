// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Core/FFactorySlot.h"
#include "Logistics/FactoryLogisticsObjectBase.h"
#include "FactoryMachineBase.generated.h"

class UFactoryRecipeBillboardComponent;
class UFactorySmartNameplateComponent;
struct FFactoryItemInstance;
class UFactoryRecipeData;
class UFactoryFacilityItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInputBufferChanged, int32, SlotIndex, FFactorySlot, SlotData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOutputBufferChanged, FFactorySlot, SlotData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentRecipeChanged, const UFactoryRecipeData*, RecipeData);
UCLASS()
class FACTORYTECHDEMO_API AFactoryMachineBase : public AFactoryLogisticsObjectBase
{
	GENERATED_BODY()

public:
	AFactoryMachineBase();
	
	UPROPERTY(BlueprintAssignable, Category = "Factory|Machine|Event")
	FOnInputBufferChanged OnInputBufferChanged;
	UPROPERTY(BlueprintAssignable, Category = "Factory|Machine|Event")
	FOnOutputBufferChanged OnOutputBufferChanged;
	UPROPERTY(BlueprintAssignable, Category = "Factory|Machine|Event")
	FOnCurrentRecipeChanged OnCurrentRecipeChanged;
	
	virtual void InitObject(const UFactoryObjectData* Data) override;
	
	virtual void PlanCycle() override;
	virtual void ExecuteCycle() override;
	virtual void UpdateView() override;
	
	virtual bool CanPushItemFromBeforeObject(
		const UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem) const override;
	
	bool IsWorking() const { return bIsWorking; }
	float GetRemainingProductionCycleTime() const { return RemainingProductionCycleTime; }
	UFactoryRecipeData* GetCurrentRecipe() const { return CurrentRecipe; }
	TArray<FFactorySlot> GetInputBufferSlots() const { return InputBufferSlots; }
	FFactorySlot GetOutputBufferSlot() const { return OutputBufferSlot; }
	
	UFUNCTION(BlueprintCallable, Category = "Factory|Machine")
	bool TryPutItemToBuffer(bool bIsInputBuffer, int32 SlotIndex, const UFactoryItemData* ItemData, int32 AmountToPut, int32& OutRemainingAmount);
	UFUNCTION(BlueprintCallable, Category = "Factory|Machine")
	bool TryTakeItemFromBuffer(bool bIsInputBuffer, int32 SlotIndex, int32 AmountToTake, FFactorySlot& OutTakenSlot);
	
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
	int32 RemainingProductionCycleTime;		// Cycle이 돌때마다 차감됨
	UPROPERTY(VisibleAnywhere, Category = "Factory|Machine")
	bool bIsWorking = false;
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|UI")
	TObjectPtr<UFactorySmartNameplateComponent> SmartNameplateComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|UI")
	TObjectPtr<UFactoryRecipeBillboardComponent> RecipeBillboardComponent;
	
private:
	void InitMachine();
	bool TryCraftItem();
	bool TryEndCraftItem();
	
	int32 InputPortIndex = 0;
	int32 OutputPortIndex = 0;
};
