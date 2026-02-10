#pragma once
#include "CoreMinimal.h"
#include "Logistics/FactoryPortComponentBase.h"
#include "FactoryInputPortComponent.generated.h"

class UFactoryItemData; 
class AFactoryLogisticsObjectBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryInputPortComponent : public UFactoryPortComponentBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Factory State")
	TObjectPtr<UFactoryItemData> PendingItem;	//설비에 이동하기위해 아이템이 예약되는 공간
    
	// 빠른 접근용 Owner 캐싱
	UPROPERTY(Transient)
	TObjectPtr<AFactoryLogisticsObjectBase> PortOwner;
	
	// PortOwner가 Item을 받을 수 있는지 확인하는 함수
	bool CanAccept(TSet<UFactoryInputPortComponent*>& Visited) const;
};