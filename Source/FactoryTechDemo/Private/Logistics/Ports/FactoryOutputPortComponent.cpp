#include "Logistics/Ports/FactoryOutputPortComponent.h"

#include "Logistics/FactoryArrowMeshComponent.h"
#include "Logistics/Ports/FactoryInputPortComponent.h"

UFactoryOutputPortComponent::UFactoryOutputPortComponent()
{
	PortArrowMeshComponent->SetArrowColor(FColor::Orange);
}

void UFactoryOutputPortComponent::BeginPlay()
{
	Super::BeginPlay();
	// 앞을 보고 InputPort을 찾음
	ScanForConnection(GetForwardVector(), UFactoryInputPortComponent::StaticClass());
}

void UFactoryOutputPortComponent::ForceScanConnection()
{
	Super::ForceScanConnection();
	ScanForConnection(GetForwardVector(), UFactoryInputPortComponent::StaticClass());
}

UFactoryInputPortComponent* UFactoryOutputPortComponent::GetConnectedInput() const
{
	// 부모 변수(ConnectedPort)를 캐스팅해서 리턴
	return Cast<UFactoryInputPortComponent>(ConnectedPort.Get());
}
