// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/HUD/FactoryWorldUIActivatorComponent.h"

#include "Player/Input/FactoryPlayerController.h"
#include "UI/World/Components/FactoryFacilityWorldUIComponent.h"


UFactoryWorldUIActivatorComponent::UFactoryWorldUIActivatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	InitSphereRadius(NormalViewRadius);
	SetGenerateOverlapEvents(false); // 수동으로 켤 것이므로 초기엔 false
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
}


void UFactoryWorldUIActivatorComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &UFactoryWorldUIActivatorComponent::OnActivatorBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UFactoryWorldUIActivatorComponent::OnActivatorEndOverlap);
}

void UFactoryWorldUIActivatorComponent::SleepAndDisable()
{
	SetUIStateForOverlappedActors(false);
	
	SetGenerateOverlapEvents(false);
}

void UFactoryWorldUIActivatorComponent::EnableAndWakeUp(EFactoryViewModeType NewViewMode)
{
	float TargetRadius = (NewViewMode == EFactoryViewModeType::TopView) ? TopViewRadius : NormalViewRadius;
	SetSphereRadius(TargetRadius);
    
	SetGenerateOverlapEvents(true);
	UpdateOverlaps();

	SetUIStateForOverlappedActors(true);
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

void UFactoryWorldUIActivatorComponent::SetUIStateForOverlappedActors(bool bWakeUp)
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors); 

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor || Actor == GetOwner()) continue;
        
		TArray<UFactoryFacilityWorldUIComponent*> UIComps;
		Actor->GetComponents<UFactoryFacilityWorldUIComponent>(UIComps);
		for (UFactoryFacilityWorldUIComponent* UIComp : UIComps)
		{
			if (bWakeUp)
			{
				UIComp->WakeUp();
			}
			else
			{
				UIComp->GoToSleep();
			}
		}
	}
}

