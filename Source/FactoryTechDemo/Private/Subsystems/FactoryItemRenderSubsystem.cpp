// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/FactoryItemRenderSubsystem.h"

#include "Items/FactoryItemRenderActor.h"

void UFactoryItemRenderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RenderActor = nullptr;
}

void UFactoryItemRenderSubsystem::Deinitialize()
{
	RenderActor = nullptr;
	Super::Deinitialize();
}

void UFactoryItemRenderSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Name = FName("ItemRenderActor_Global");
	
	RenderActor = InWorld.SpawnActor<AFactoryItemRenderActor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (RenderActor)
	{
		RenderActor->InitializeRenderers();
	}
}

void UFactoryItemRenderSubsystem::RequestRenderItem(const UFactoryItemData* ItemData, const FTransform& Transform)
{
	if (RenderActor)
	{
		RenderActor->RequestRenderItem(ItemData, Transform);
	}
}

void UFactoryItemRenderSubsystem::RegisterActiveBelt(AFactoryBelt* Belt)
{
	if (RenderActor)
	{
		RenderActor->RegisterActiveBelt(Belt);
	}
}

void UFactoryItemRenderSubsystem::UnregisterActiveBelt(AFactoryBelt* Belt)
{
	if (RenderActor)
	{
		RenderActor->UnRegisterActiveBelt(Belt);
	}
}
