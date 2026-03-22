#pragma once
#include "CoreMinimal.h"
#include "Items/FactoryItemData.h"
#include "Logistics/FactoryPortComponentBase.h"
#include "FactoryInputPortComponent.generated.h"

class UFactoryOutputPortComponent;
class AFactoryLogisticsObjectBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryInputPortComponent : public UFactoryPortComponentBase
{
	GENERATED_BODY()

public:
	UFactoryInputPortComponent();
	
	virtual void BeginPlay() override;

	UPROPERTY(Transient, SkipSerialization)
	FFactoryItemInstance PendingItem;	//설비에 이동하기위해 아이템이 예약되는 공간
	
	UFactoryOutputPortComponent* GetConnectedOutput() const;
};