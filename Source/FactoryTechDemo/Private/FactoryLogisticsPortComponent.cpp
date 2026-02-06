// Fill out your copyright notice in the Description page of Project Settings.


#include "FactoryLogisticsPortComponent.h"

#include "FactoryBuildingSettings.h"


// Sets default values for this component's properties
UFactoryLogisticsPortComponent::UFactoryLogisticsPortComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	BoxExtent = FVector(10.0f, 40.0f, 10.0f);
	AddRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
}

bool UFactoryLogisticsPortComponent::TryConnect(UFactoryLogisticsPortComponent* TargetNode)
{
	if (TargetNode == nullptr) return false;
	if (TargetNode->PortType == PortType)
	{
		return false;
	}
	ConnectedPort = TargetNode;
	return true;
}

bool UFactoryLogisticsPortComponent::ScanForConnection()
{
	FVector Start = GetComponentLocation();
	FVector Direction = PortType == EFactoryPortType::Output ? GetForwardVector() : -GetForwardVector();
	const UFactoryBuildingSettings* BuildingSettings = GetDefault<UFactoryBuildingSettings>();
	FVector End = Start + (Direction * BuildingSettings->GetGridLength() * 0.5f);
	
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_GameTraceChannel2, Params))
	{
		if (UFactoryLogisticsPortComponent* TargetPort = Cast<UFactoryLogisticsPortComponent>(HitResult.GetComponent()))
		{
			if (TargetPort->PortType != PortType)
			{
				if (TargetPort->TryConnect(this))
				{
					ConnectedPort = TargetPort;
					return true;
				}
			}
		}
	}
	return false;
}

// Called when the game starts
void UFactoryLogisticsPortComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UFactoryLogisticsPortComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

