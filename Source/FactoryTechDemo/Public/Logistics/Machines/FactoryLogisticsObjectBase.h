#pragma once

#include "CoreMinimal.h"
#include "Placement/FactoryPlaceObjectBase.h"
#include "Items/FactoryItemData.h"
#include "FactoryLogisticsObjectBase.generated.h"

class UFactoryObjectData;
class UFactoryInputPortComponent;
class UFactoryOutputPortComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFacilityBlockedStateChanged, bool, bIsBlocked);

UENUM(BlueprintType)
enum class ELogisticsObjectType : uint8
{
	Conveyor,
	Facility,
};

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
	
	// 서브시스템에서 호출하는 사이클 내 페이즈 메서드	// TODO : 자식 클래스에서 구현 및 겹치는 부분 많다면 Base 클래스에서 일부 구현
	// 오브젝트 내부 플래그 변수 등을 초기화하는 단계
	virtual void InitPhase() {};
	// 물류간 이동 로직이 시행됨
	virtual void LogisticsPhase() PURE_VIRTUAL(AFactoryLogisticsObjectBase::LogisticsPhase, ; );
	// 순환참조 등으로 LogisticsPhase에서 처리되지 못한 작업을 처리함
	virtual void LateLogisticsPhase() {};
	// 현재 설비의 InputPort에 Pending되어있는 아이템을 설비로 가져옴
	virtual void LogicPhase() PURE_VIRTUAL(AFactoryLogisticsObjectBase::LogicPhase, ; );
	// Cycle 결과에 따른 비주얼적 업데이트 실현
	virtual void VisualPhase() PURE_VIRTUAL(AFactoryLogisticsObjectBase::VisualPhase, ; );
	
	// 아이템 타입을 확인해 받을 수 있는지 판단하는 함수
	virtual bool CanReceiveItem(UFactoryInputPortComponent* RequestPort, const UFactoryItemData* IncomingItem) PURE_VIRTUAL(AFactoryLogisticsObjectBase::CanPushItemFromBeforeObject, return false; );
	virtual const UFactoryItemData* PeekOutputItem(UFactoryOutputPortComponent* RequestPort) {return nullptr;}
	virtual FFactoryItemInstance ConsumeItem(UFactoryOutputPortComponent* RequestPort) { return FFactoryItemInstance(); }
	
	// 분배기의 라운드로빈 로직 구현을 위해 사용. 일반적인 물류이동을 위해서는 사용하면 안됨
	virtual void ForceAcceptPushedItem(UFactoryInputPortComponent* RequestPort, FFactoryItemInstance Item)
	{
		ReceiveItem(RequestPort, Item);
	};
	
	int32 GetConnectedOutputPortNumber() const;
	TArray<AFactoryLogisticsObjectBase*> GetConnectedInputPortsObject() const;
	
	ELogisticsObjectType GetLogisticsObjectType() const { return LogisticsObjectType; }
	const TArray<UFactoryOutputPortComponent*> GetOutputPorts() const  { return LogisticsOutputPortArr; }
	const TArray<UFactoryInputPortComponent*> GetInputPorts() const  { return LogisticsInputPortArr; }

protected:
	// 포트 자동 수집 및 분류 함수
	void InitializeLogisticsPort();
	
	// 아이템을 받아 처리하는 함수 구현. 상속을 받는 설비에 따라 버퍼 공간에 넣거나 가공을 하는 등 행동을 정의 해야함.
	virtual void ReceiveItem(UFactoryInputPortComponent* RequestPort, FFactoryItemInstance Item) PURE_VIRTUAL(AFactoryLogisticsObjectBase::ReceiveItem, ; );
	
	UFUNCTION(BlueprintCallable, Category = "Factory|State")
	void SetFacilityBlocked(bool bNewBlock);
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UFactoryOutputPortComponent>> LogisticsOutputPortArr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFactoryInputPortComponent>> LogisticsInputPortArr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factory|State")
	bool bIsFacilityBlocked = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factory|LogisticsType")
	ELogisticsObjectType LogisticsObjectType = ELogisticsObjectType::Conveyor;
};