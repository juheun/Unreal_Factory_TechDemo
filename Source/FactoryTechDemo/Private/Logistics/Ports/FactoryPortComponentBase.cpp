#include "Logistics/Ports/FactoryPortComponentBase.h"

#include "Components/ArrowComponent.h"
#include "Logistics/Machines/FactoryLogisticsObjectBase.h"
#include "Core/FactoryDeveloperSettings.h"

UFactoryPortComponentBase::UFactoryPortComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	BoxExtent = FVector(10.0f, 40.0f, 10.0f);
	
	PortDirArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	PortDirArrowComponent->SetupAttachment(this);
	PortDirArrowComponent->SetHiddenInGame(false);
	PortDirArrowComponent->SetRelativeLocation(FVector(-40.0f, 0.f, 0.f));
	PortDirArrowComponent->SetArrowLength(80.f);
}

void UFactoryPortComponentBase::BeginPlay()
{
	Super::BeginPlay();
	
	ConnectedPort = nullptr;
	
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
			TargetPort->PortDirArrowComponent->SetHiddenInGame(false);
			TargetPort->OnPortConnectionChanged.Broadcast(TargetPort, false);
		}
		ConnectedPort = nullptr;
		PortDirArrowComponent->SetHiddenInGame(false);
		OnPortConnectionChanged.Broadcast(this, false);
	}
}

void UFactoryPortComponentBase::SetPortBlocked(bool bNewBlocked)
{
	if (bIsPortBlocked != bNewBlocked)
	{
		bIsPortBlocked = bNewBlocked;
		OnPortBlockedStateChanged.Broadcast(bIsPortBlocked);
	}
}

void UFactoryPortComponentBase::SetPortEnabled(bool bEnabled)
{
	if (bEnabled == bIsPortEnabled) return;
	
	bIsPortEnabled = bEnabled;
	
	// 1. 콜리전 제어
	SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    
	// 2. 화살표 시각적 제어 (켜지면 화살표 보임, 꺼지면 화살표 숨김)
	// 단, 포트가 '연결(Connected)'된 상태라면 무조건 숨겨야 하므로 조건 추가
	bool bShouldHideArrow = !bEnabled || ConnectedPort.IsValid();
	PortDirArrowComponent->SetHiddenInGame(bShouldHideArrow);
	
	if (bEnabled)
	{
		ForceScanConnection();
	}
	else
	{
		Disconnect();
	}
}

AFactoryLogisticsObjectBase* UFactoryPortComponentBase::GetPortOwner()
{
	if (AFactoryLogisticsObjectBase* TmpPortOwner = PortOwner.Get())
	{
		return TmpPortOwner;
	}

	// 만약 비어있다면 (BeginPlay 전이거나 캐싱 누락 시), 한 번만 Cast 연산 수행 후 캐싱
	PortOwner = Cast<AFactoryLogisticsObjectBase>(GetOwner());
	
	return PortOwner.Get();
}

void UFactoryPortComponentBase::ConnectTo(UFactoryPortComponentBase* Target)
{
	if (!Target) return;
	
	AFactoryLogisticsObjectBase* MyOwner = GetPortOwner();
	AFactoryLogisticsObjectBase* TargetOwner = Target->GetPortOwner();
	
	if (MyOwner && TargetOwner)
	{
		// 기획상 설비와 설비의 직결은 불가능함
		if (TargetOwner->GetLogisticsObjectType() == ELogisticsObjectType::Facility && 
			MyOwner->GetLogisticsObjectType() == ELogisticsObjectType::Facility)
		{
			return;
		}
	}
	
	// 상호 연결 (Handshake)
	this->ConnectedPort = Target;
	PortDirArrowComponent->SetHiddenInGame(true);
	OnPortConnectionChanged.Broadcast(this, true);
	
	Target->ConnectedPort = this;
	Target->PortDirArrowComponent->SetHiddenInGame(true);
	Target->OnPortConnectionChanged.Broadcast(Target, true);
}

void UFactoryPortComponentBase::ScanForConnection(FVector Direction, TSubclassOf<UFactoryPortComponentBase> TargetClassType)
{
	if (ConnectedPort.Get() || !bIsPortEnabled ) return; // 이미 연결됐으면 스캔 X

	float GridSize = GetDefault<UFactoryDeveloperSettings>()->GetGridLength();
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