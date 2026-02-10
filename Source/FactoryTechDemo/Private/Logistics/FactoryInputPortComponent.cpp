#include "Logistics/FactoryInputPortComponent.h"

#include "Logistics/FactoryLogisticsObjectBase.h"
#include "Logistics/FactoryOutputPortComponent.h"

void UFactoryInputPortComponent::BeginPlay()
{
	Super::BeginPlay();
	
	PortOwner = Cast<AFactoryLogisticsObjectBase>(GetOwner());

	ScanForConnection(-GetForwardVector(), UFactoryOutputPortComponent::StaticClass());
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