// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryLogisticsObjectBase.h"
#include "Logistics/FactoryLogisticsTypes.h"
#include "FactoryBelt.generated.h"

class USplineComponent;
class UFactoryItemData;

UCLASS()
class FACTORYTECHDEMO_API AFactoryBelt : public AFactoryLogisticsObjectBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFactoryBelt();
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void PlanCycle() override;
	virtual void ExecuteCycle() override;
	virtual void UpdateView() override;
	virtual void Tick(float DeltaSeconds) override;
	
	virtual bool CanPushItemFromBeforeObject(
		UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem) override;
	
	void SetBeltType(EBeltType Type);

	// 벨트의 타입을 고려한 실제 배출 방향을 반환
	FVector GetBeltExitDirection() const;
	EBeltType GetBeltType() const {return BeltType;};

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory|Movement")
	EBeltType BeltType;
	
	UPROPERTY(EditDefaultsOnly, Category = "Factory|Movement")
	TMap<EBeltType, TObjectPtr<UStaticMesh>> BeltMeshMap;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory|Movement")
	TObjectPtr<USplineComponent> SplineComponent;
	
	UPROPERTY()
	FFactoryItemInstance CurrentItem;
	
	void UpdateSplinePath(EBeltType Type);
	void UpdateBeltVisual(EBeltType Type);
	void SetSpineDistance(float Alpha);
	virtual bool PullItemFromInputPorts(FFactoryItemInstance& Item) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	const float BeltHeight = 20.f;
	bool bIsBeltStop = false;
	float TotalSpineLength = 0.f;
};
