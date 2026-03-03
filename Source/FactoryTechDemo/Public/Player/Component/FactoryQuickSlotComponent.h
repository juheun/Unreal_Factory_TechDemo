// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactoryQuickSlotComponent.generated.h"

class AFactoryPlayerController;
class UFactoryInventoryWidget;
class UFactoryObjectData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryQuickSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFactoryQuickSlotComponent();
	
	UFactoryObjectData* GetQuickSlotData(int Index);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Factory|Data")
	TArray<TObjectPtr<UFactoryObjectData>> QuickSlotObjectDataArr;
	
private:
	UPROPERTY()
	TWeakObjectPtr<AFactoryPlayerController> CachedPlayerController;
};
