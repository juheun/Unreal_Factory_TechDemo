#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "FactoryPortComponentBase.generated.h"

class AFactoryLogisticsObjectBase;

UCLASS(Abstract)
class FACTORYTECHDEMO_API UFactoryPortComponentBase : public UBoxComponent
{
	GENERATED_BODY()

public:
	UFactoryPortComponentBase();
	
	virtual void BeginPlay() override;
	// 철거 시 연결 끊기 위해 오버라이드
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 연결 함수
	void ConnectTo(UFactoryPortComponentBase* Target);
	void Disconnect();
	
	AFactoryLogisticsObjectBase* GetPortOwner() const { return PortOwner;}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Logistics")
	TObjectPtr<AFactoryLogisticsObjectBase> PortOwner;
	// 내가 연결된 대상
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Logistics")
	TObjectPtr<UFactoryPortComponentBase> ConnectedPort;
	
	// 연결 스캔 로직
	void ScanForConnection(FVector Direction, TSubclassOf<UFactoryPortComponentBase> TargetClassType);
};