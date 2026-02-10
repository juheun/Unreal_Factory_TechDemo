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
	TObjectPtr<UFactoryItemData> CurrentItem;
    
	UPROPERTY(VisibleAnywhere, Category = "Factory State")
	TObjectPtr<UFactoryItemData> PendingItem;
    
	// 빠른 접근용 Owner 캐싱
	UPROPERTY(Transient)
	TObjectPtr<AFactoryLogisticsObjectBase> OwningMachine;

	bool CanAccept(TSet<UFactoryInputPortComponent*>& Visited);
};