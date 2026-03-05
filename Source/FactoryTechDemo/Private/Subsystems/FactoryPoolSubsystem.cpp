// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/FactoryPoolSubsystem.h"


void UFactoryPoolSubsystem::ReturnItemToPool(AActor* Actor)
{
	if (!Actor) return;
	
	Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	PoolMap.FindOrAdd(Actor->GetClass()).InactiveItems.Push(Actor);
}

AActor* UFactoryPoolSubsystem::Internal_GetItem(TSubclassOf<AActor> ClassType, const FVector& Location, const FRotator& Rotation)
{
	FFactoryPoolStack& ItemStack = PoolMap.FindOrAdd(ClassType);
	AActor* Actor = nullptr;
	
	if (ItemStack.InactiveItems.Num() > 0)
	{
		Actor = ItemStack.InactiveItems.Pop();
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Actor = GetWorld()->SpawnActor<AActor>(ClassType, Location, Rotation, SpawnParams);
	}
	
	if (!Actor) return nullptr;
	
	Actor->SetActorLocationAndRotation(Location, Rotation);
	Actor->SetActorHiddenInGame(false);
	return Actor;
}
