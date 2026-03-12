// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactoryQuickSlotComponent.generated.h"

class UFactoryQuickBarWidget;
class UFactoryInputConfig;
class AFactoryPlayerController;
class UFactoryInventoryWidget;
class UFactoryObjectData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuickSlotDataChangedSignature, int32, Index, UFactoryObjectData*, ObjectData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuickSlotExecuteSignature, UFactoryObjectData*, ObjectData);
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryQuickSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFactoryQuickSlotComponent();
	void SetUpInputComponent(UEnhancedInputComponent* PlayerInputComp, const UFactoryInputConfig* InputConfig);
	
	UPROPERTY(BlueprintAssignable)
	FOnQuickSlotExecuteSignature OnQuickSlotExecuted;
	UPROPERTY(BlueprintAssignable)
	FOnQuickSlotDataChangedSignature OnQuickSlotDataChanged;
	
	void SetQuickSlotData(int32 Index, UFactoryObjectData* InData);

	const TArray<TObjectPtr<UFactoryObjectData>>& GetQuickSlotDataArray() const {return QuickSlotObjectDataArr;}
	
protected:
	virtual void BeginPlay() override;
	void ExecuteQuickSlotData(int Index);
	
	UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
	TSubclassOf<UFactoryQuickBarWidget> QuickBarWidgetBP;
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|UI")
	TObjectPtr<UFactoryQuickBarWidget> QuickBarWidget;
	
	UPROPERTY(EditAnywhere, Category = "Factory|Data")
	TArray<TObjectPtr<UFactoryObjectData>> QuickSlotObjectDataArr;
	
private:
	UPROPERTY()
	TWeakObjectPtr<AFactoryPlayerController> CachedPlayerController;
};
