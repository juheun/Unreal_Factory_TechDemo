#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "FactoryPortComponentBase.generated.h"

class UArrowComponent;
class AFactoryLogisticsObjectBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPortConnectionChangedSignature, UFactoryPortComponentBase*, Port, bool, bIsConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPortBlockedStateChanged, bool, bIsBlocked);
UCLASS(Abstract)
class FACTORYTECHDEMO_API UFactoryPortComponentBase : public UBoxComponent
{
	GENERATED_BODY()

public:
	UFactoryPortComponentBase();
	
	UPROPERTY(BlueprintAssignable, Category="Factory|Port|Event")
	FOnPortBlockedStateChanged OnPortBlockedStateChanged;
	UPROPERTY(BlueprintAssignable, Category="Factory|Port|Event")
	FOnPortConnectionChangedSignature OnPortConnectionChanged;
	
	virtual void BeginPlay() override;
	// 철거 시 연결 끊기 위해 오버라이드
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 연결 함수
	void ConnectTo(UFactoryPortComponentBase* Target);
	void Disconnect();
	
	void SetPortBlocked(bool bNewBlocked);
	void SetPortEnabled(bool bEnabled);
	
	AFactoryLogisticsObjectBase* GetPortOwner() const { return PortOwner.Get();}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Logistics")
	TWeakObjectPtr<AFactoryLogisticsObjectBase> PortOwner;
	// 내가 연결된 대상
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Logistics")
	TWeakObjectPtr<UFactoryPortComponentBase> ConnectedPort;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Logistics")
	TObjectPtr<UArrowComponent> PortDirArrowComponent;
	
	// 연결 스캔 로직
	void ScanForConnection(FVector Direction, TSubclassOf<UFactoryPortComponentBase> TargetClassType);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factory|Port|State")
	bool bIsPortBlocked = false;
};