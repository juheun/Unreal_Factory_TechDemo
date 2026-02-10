#include "Logistics/FactoryInputPortComponent.h"

#include "Logistics/FactoryLogisticsObjectBase.h"
#include "Logistics/FactoryOutputPortComponent.h" // cpp에는 include 필수

void UFactoryInputPortComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwningMachine = Cast<AFactoryLogisticsObjectBase>(GetOwner());

	ScanForConnection(-GetForwardVector(), UFactoryOutputPortComponent::StaticClass());
}

bool UFactoryInputPortComponent::CanAccept(TSet<UFactoryInputPortComponent*>& Visited)
{
	if (OwningMachine && !OwningMachine->CanReceiveItem(this))
	{
		return false;
	}
    
	// ... 재귀 로직 ...
	return false; // 임시 리턴
}