// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "FactoryLogisticsPortComponent.generated.h"

UENUM(BlueprintType)
enum class EFactoryPortType : uint8 {Input, Output};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryLogisticsPortComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFactoryLogisticsPortComponent();
	
	UPROPERTY(EditDefaultsOnly, Category="Logistics")
	EFactoryPortType PortType;
	
	UPROPERTY(VisibleAnywhere, Category="Logistics")
	TObjectPtr<UFactoryLogisticsPortComponent> ConnectedPort;
	
	UPROPERTY()
	TObjectPtr<class AFactoryLogisticsObjectBase> PortOwner;
	
	bool TryConnect(UFactoryLogisticsPortComponent* TargetNode);
	bool ScanForConnection();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
};
