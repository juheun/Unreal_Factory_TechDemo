#include "Logistics/FactoryInputPortComponent.h"

#include "Logistics/FactoryLogisticsObjectBase.h"
#include "Logistics/FactoryOutputPortComponent.h"

void UFactoryInputPortComponent::BeginPlay()
{
	Super::BeginPlay();
	// 뒤를 보고 OutputPort를 찾음
	ScanForConnection(-GetForwardVector(), UFactoryOutputPortComponent::StaticClass());
}

UFactoryOutputPortComponent* UFactoryInputPortComponent::GetConnectedOutput() const
{
	// 부모 변수(ConnectedPort)를 캐스팅해서 리턴
	return Cast<UFactoryOutputPortComponent>(ConnectedPort);
}

bool UFactoryInputPortComponent::CanAccept(TSet<UFactoryInputPortComponent*>& Visited) const
{
	if (PortOwner && !PortOwner->CanPushItemFromBeforeObject(this))
	{
		return false;
	}
    
	// ... 재귀 로직 ...
	return false; // 임시 리턴
}
