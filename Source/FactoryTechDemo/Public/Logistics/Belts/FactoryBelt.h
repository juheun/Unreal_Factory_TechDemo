// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Logistics/Machines/FactoryLogisticsObjectBase.h"
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
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void InitPhase() override;
	virtual void LogisticsPhase() override;
	virtual void LateLogisticsPhase() override;
	virtual void LogicPhase() override;
	virtual void VisualPhase() override;
	
	void SetBeltType(EBeltType Type);
	
	virtual bool TryGetInteractionOptions(const EPlacementMode CurrentMode, TArray<FInteractionOption>& OutOptions) const override;
	virtual void Interact(const AActor* Interactor, const EPlacementMode CurrentMode, int32 OptionIndex = 0) override;
	
	virtual const UFactoryItemData* PeekOutputItem(UFactoryOutputPortComponent* RequestPort) override;
	virtual FFactoryItemInstance ConsumeItem(UFactoryOutputPortComponent* RequestPort) override;
	virtual bool CanReceiveItem(UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem) override;
	
	TSet<AFactoryBelt*> GetConnectedBeltLine();
	
	FVector GetBeltExitDirection() const;	// 벨트의 타입을 고려한 실제 배출 방향을 반환
	FTransform GetItemRenderTransform(float CycleAlpha) const;
	EBeltType GetBeltType() const {return BeltType;}
	const UFactoryItemData* GetCurrentItemData() const { return CurrentItem.ItemData.Get(); }

protected:
	virtual void ReceiveItem(UFactoryInputPortComponent* RequestPort, FFactoryItemInstance Item) override;
	
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
	FTransform GetSpineDistance(float Alpha) const;
	void CacheSplineTransforms();
	
	void MassRetrieve();
	
private:
	void TryPullInputFromPorts();
	
	UPROPERTY(Transient)
	TArray<FTransform> CachedSplineTransforms;
	const int32 SplineCacheSteps = 10;	//CachedSplineTransforms에 저장할 스플라인 포인트 수
	
	const float BeltHeight = 20.f;
	bool bIsBeltStop = false;
	UPROPERTY()
	float TotalSpineLength = 0.f;
	bool bReceivedThisCycle = false;
	bool bIsBeltRegistered = false;		// 아이템 랜더 액터에 등록되었는지에 대한 내부 캐싱
};
