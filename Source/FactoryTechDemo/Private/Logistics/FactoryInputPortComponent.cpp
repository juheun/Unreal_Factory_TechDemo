#include "Logistics/FactoryInputPortComponent.h"

#include "Components/ArrowComponent.h"
#include "Logistics/FactoryOutputPortComponent.h"

UFactoryInputPortComponent::UFactoryInputPortComponent()
{
	PortDirArrowComponent->ArrowColor = FColor::Green;
}

void UFactoryInputPortComponent::BeginPlay()
{
	Super::BeginPlay();
	// 뒤를 보고 OutputPort를 찾음
	ScanForConnection(-GetForwardVector(), UFactoryOutputPortComponent::StaticClass());
}

UFactoryOutputPortComponent* UFactoryInputPortComponent::GetConnectedOutput() const
{
	// 부모 변수(ConnectedPort)를 캐스팅해서 리턴
	return Cast<UFactoryOutputPortComponent>(ConnectedPort.Get());
}