#pragma once

#include "CoreMinimal.h"
#include "Placement/FactoryPlaceObjectBase.h"
#include "Items/FactoryItemData.h"
#include "FactoryLogisticsObjectBase.generated.h"

class UFactoryObjectData;
class UFactoryInputPortComponent;
class UFactoryOutputPortComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFacilityBlockedStateChanged, bool, bIsBlocked);

UCLASS(Abstract)
class FACTORYTECHDEMO_API AFactoryLogisticsObjectBase : public AFactoryPlaceObjectBase
{
	GENERATED_BODY()

public:
	AFactoryLogisticsObjectBase();
	
	UPROPERTY(BlueprintAssignable, Category = "Factory|Event")
	FOnFacilityBlockedStateChanged OnFacilityBlockedStateChanged;
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// 서브시스템에서 호출하는 3단계 사이클	// TODO : 자식 클래스에서 구현 및 겹치는 부분 많다면 Base 클래스에서 일부 구현
	// 현재 설비에서 다음 설비의 InputPort로 아이템을 Pending 할 수 있는지 확인 및 가능시 Pending
	virtual void PlanCycle() PURE_VIRTUAL(AFactoryLogisticsObjectBase::PlanCycle, ; );
	// 순환참조 등으로 PlanCycle에서 처리되지 못한 작업을 처리함
	virtual void LatePlanCycle() {};
	// 현재 설비의 InputPort에 Pending되어있는 아이템을 설비로 가져옴
	virtual void ExecuteCycle() PURE_VIRTUAL(AFactoryLogisticsObjectBase::ExecuteCycle, ; );
	// Cycle 결과에 따른 비주얼적 업데이트 실현
	virtual void UpdateView() PURE_VIRTUAL(AFactoryLogisticsObjectBase::UpdateView, ; );
    
	// 이전 설비에서 InputPort로 아이템을 밀어넣을 수 있는지 확인해주는 로직을 구현
	virtual bool CanPushItemFromBeforeObject(
		UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem) 
		PURE_VIRTUAL(AFactoryLogisticsObjectBase::CanPushItemFromBeforeObject, return false; );
	
	int32 GetConnectedOutputPortNumber() const;
	TArray<AFactoryLogisticsObjectBase*> GetConnectedInputPortsObject() const;

protected:
	// 포트 자동 수집 및 분류 함수
	void InitializeLogisticsPort();
	virtual void OnConstruction(const FTransform& Transform) override;
	
	// InputPort에 Pending된 아이템을 처리하는 함수 구현. 상속을 받는 설비에 따라 버퍼 공간에 넣거나 가공을 하는 등 행동을 정의 해야함.
	virtual bool PullItemFromInputPorts(FFactoryItemInstance& Item) 
		PURE_VIRTUAL(AFactoryLogisticsObjectBase::ReceiveItem, return false; );
	
	UFUNCTION(BlueprintCallable, Category = "Factory|State")
	void SetFacilityBlocked(bool bNewBlock);
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UFactoryOutputPortComponent>> LogisticsOutputPortArr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFactoryInputPortComponent>> LogisticsInputPortArr;
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factory|State")
	bool bIsFacilityBlocked = false;
};