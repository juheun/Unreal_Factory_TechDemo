// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryLogisticsObjectBase.h"
#include "FactoryBelt.generated.h"

class USplineComponent;
class UFactoryItemData;

UENUM(BlueprintType)
enum class EBeltType : uint8 { Straight, LeftTurn, RightTurn};

UCLASS()
class FACTORYTECHDEMO_API AFactoryBelt : public AFactoryLogisticsObjectBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFactoryBelt();
	
	virtual void PlanCycle() override;
	virtual void ExecuteCycle() override;
	virtual void UpdateView() override;
	virtual void Tick(float DeltaSeconds) override;
	
	virtual bool CanPushItemFromBeforeObject(const UFactoryInputPortComponent* RequestPort) const override;
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	EBeltType BeltType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<USplineComponent> SplineComponent;
	
	UPROPERTY()
	FFactoryItemInstance CurrentItem;
	
	void UpdateSplinePath(EBeltType Type);
	void SetSpineDistance(float Alpha);
	virtual bool PullItemFromInputPorts(FFactoryItemInstance& Item) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	const float BeltHeight = 20.f;
	bool bIsBeltStop = false;
	float TotalSpineLength = 0.f;
};
