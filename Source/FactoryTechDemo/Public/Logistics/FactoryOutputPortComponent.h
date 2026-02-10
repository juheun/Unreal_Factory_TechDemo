#pragma once
#include "CoreMinimal.h"
#include "Logistics/FactoryPortComponentBase.h"
#include "FactoryOutputPortComponent.generated.h"

class UFactoryInputPortComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryOutputPortComponent : public UFactoryPortComponentBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// 연결된 놈을 Input으로 캐스팅해서 줌
	UFactoryInputPortComponent* GetConnectedInput() const;
};