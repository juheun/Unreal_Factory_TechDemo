#include "Logistics/FactoryPortComponentBase.h"

#include "Logistics/FactoryLogisticsObjectBase.h"
#include "Settings/FactoryBuildingSettings.h"

UFactoryPortComponentBase::UFactoryPortComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	BoxExtent = FVector(10.0f, 40.0f, 10.0f);
}

void UFactoryPortComponentBase::BeginPlay()
{
	Super::BeginPlay();
	
	PortOwner = Cast<AFactoryLogisticsObjectBase>(GetOwner());
}

void UFactoryPortComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 내가 파괴될 때 연결된 포트에 연결 해제 알림
	Disconnect();
	Super::EndPlay(EndPlayReason);
}

void UFactoryPortComponentBase::Disconnect()
{
	if (UFactoryPortComponentBase* TargetPort = ConnectedPort.Get())
	{
		// 상대방의 연결 정보도 지워줌 (상호 해제)
		if (TargetPort->ConnectedPort.Get() == this)
		{
			TargetPort->ConnectedPort = nullptr;
		}
		ConnectedPort = nullptr;
	}
}

void UFactoryPortComponentBase::ConnectTo(UFactoryPortComponentBase* Target)
{
	if (!Target) return;

	// 상호 연결 (Handshake)
	this->ConnectedPort = Target;
	Target->ConnectedPort = this;
}

void UFactoryPortComponentBase::ScanForConnection(FVector Direction, TSubclassOf<UFactoryPortComponentBase> TargetClassType)
{
	if (ConnectedPort.Get()) return; // 이미 연결됐으면 스캔 X

	float GridSize = GetDefault<UFactoryBuildingSettings>()->GetGridLength();
	FVector Start = GetComponentLocation();
	FVector End = Start + (Direction * GridSize * 0.6f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel2, Params))
	{
		auto* HitPort = Cast<UFactoryPortComponentBase>(Hit.GetComponent());
        
		// 내가 찾는 타입(TargetClassType)이 맞는지 확인
		if (HitPort && HitPort->IsA(TargetClassType))
		{
			ConnectTo(HitPort);
		}
	}
}