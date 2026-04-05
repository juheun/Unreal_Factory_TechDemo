// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/HUD/FactoryWorldUIActivatorComponent.h"

#include "Player/Input/FactoryPlayerController.h"
#include "UI/World/Components/FactoryFacilityWorldUIComponent.h"


UFactoryWorldUIActivatorComponent::UFactoryWorldUIActivatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	InitSphereRadius(NormalViewRadius);
	SetGenerateOverlapEvents(true);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
}


void UFactoryWorldUIActivatorComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &UFactoryWorldUIActivatorComponent::OnActivatorBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UFactoryWorldUIActivatorComponent::OnActivatorEndOverlap);

	// 뷰 모드 전환 델리게이트 구독
	if (AFactoryPlayerController* PC = Cast<AFactoryPlayerController>(GetOwner()))
	{
		PC->OnViewModeChanged.AddDynamic(this, &UFactoryWorldUIActivatorComponent::OnViewModeChanged);
		OnViewModeChanged(PC->GetCurrentViewMode());
	}
	
	// 스폰 시점에 반경 내에 들어와있는 UFactoryFacilityWorldUIComponent들을 깨워줌
	TArray<UPrimitiveComponent*> OverlappingComps;
	GetOverlappingComponents(OverlappingComps);
	for (UPrimitiveComponent* Comp : OverlappingComps)
	{
		if (AActor* OverlappedActor = Comp->GetOwner())
		{
			TArray<UFactoryFacilityWorldUIComponent*> UIComps;
			OverlappedActor->GetComponents<UFactoryFacilityWorldUIComponent>(UIComps);
			for (auto* UIComp : UIComps)
			{
				UIComp->WakeUp();
			}
		}
	}
}

void UFactoryWorldUIActivatorComponent::OnActivatorBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner()) return;

	TArray<UFactoryFacilityWorldUIComponent*> UIComps;
	OtherActor->GetComponents<UFactoryFacilityWorldUIComponent>(UIComps);

	for (UFactoryFacilityWorldUIComponent* Comp : UIComps)
	{
		Comp->WakeUp();
	}
}

void UFactoryWorldUIActivatorComponent::OnActivatorEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                                               AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == GetOwner()) return;

	TArray<UFactoryFacilityWorldUIComponent*> UIComps;
	OtherActor->GetComponents<UFactoryFacilityWorldUIComponent>(UIComps);

	for (UFactoryFacilityWorldUIComponent* Comp : UIComps)
	{
		Comp->GoToSleep();
	}
}

void UFactoryWorldUIActivatorComponent::OnViewModeChanged(EFactoryViewModeType NewViewMode)
{
	if (NewViewMode == EFactoryViewModeType::TopView)
	{
		SetSphereRadius(TopViewRadius);
	}
	else
	{
		SetSphereRadius(NormalViewRadius);
	}
}

