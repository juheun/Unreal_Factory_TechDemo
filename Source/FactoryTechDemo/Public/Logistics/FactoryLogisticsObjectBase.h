#pragma once

#include "CoreMinimal.h"
#include "FactoryPlaceObjectBase.h"
#include "FactoryLogisticsObjectBase.generated.h"

class UFactoryItemData;
class UFactoryObjectData;
class UFactoryInputPortComponent;
class UFactoryOutputPortComponent;

UCLASS(Abstract)
class FACTORYTECHDEMO_API AFactoryLogisticsObjectBase : public AFactoryPlaceObjectBase
{
	GENERATED_BODY()

public:
	AFactoryLogisticsObjectBase();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void InitObject(const UFactoryObjectData* Data) override;

	// 서브시스템에서 호출하는 3단계 사이클
	virtual void DetermineNextState();
	virtual void CommitState();
	virtual void UpdateState();
    
	// 자식 클래스에서 이 포트로 받아도 되는지 판단하는 로직을 구현
	virtual bool CanReceiveItem(const UFactoryInputPortComponent* RequestPort) const PURE_VIRTUAL(AFactoryLogisticsObjectBase::CanReceiveItem, return false; );
    
	// 아이템이 실제로 도착했을 때 호출
	virtual void ReceiveItem(const UFactoryItemData* Item) PURE_VIRTUAL(AFactoryLogisticsObjectBase::ReceiveItem, ; );

protected:
	// 포트 자동 수집 및 분류 함수
	void InitializeLogisticsPort();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Logistics")
	TArray<TObjectPtr<UFactoryOutputPortComponent>> LogisticsOutputPortArr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Logistics")
	TArray<TObjectPtr<UFactoryInputPortComponent>> LogisticsInputPortArr;
};