#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "FactoryPortComponentBase.generated.h"

UCLASS(Abstract)
class FACTORYTECHDEMO_API UFactoryPortComponentBase : public UBoxComponent
{
	GENERATED_BODY()

public:
	UFactoryPortComponentBase();

	// 철거 시 연결 끊기 위해 오버라이드
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 내가 연결된 대상
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Logistics")
	TObjectPtr<UFactoryPortComponentBase> ConnectedPort;

	// 연결/해제 함수
	void ConnectTo(UFactoryPortComponentBase* Target);
	void Disconnect();

protected:
	// 스캔 로직
	void ScanForConnection(FVector Direction, TSubclassOf<UFactoryPortComponentBase> TargetClassType);
};