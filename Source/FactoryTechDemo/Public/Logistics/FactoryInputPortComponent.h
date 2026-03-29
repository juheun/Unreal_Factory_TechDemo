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
	virtual void ForceScanConnection() override;
	
	UFactoryOutputPortComponent* GetConnectedOutput() const;
};