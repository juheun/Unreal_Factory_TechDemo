// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/FactoryPoolSubsystem.h"

#include "Items/FactoryItemVisual.h"
#include "Settings/FactoryDeveloperSettings.h"

AFactoryItemVisual* UFactoryPoolSubsystem::GetItemVisualFromPool(
	const UFactoryItemData* ItemData, const FVector& Location, const FRotator& Rotation)
{
	AFactoryItemVisual* ItemVisual;
	if (InactiveItemVisuals.Num() > 0)
	{
		ItemVisual = InactiveItemVisuals.Pop();
	}
	else
	{
		const UFactoryDeveloperSettings* DeveloperSettings = GetDefault<UFactoryDeveloperSettings>();
		TSubclassOf<AFactoryItemVisual> VisualBP = DeveloperSettings->GetItemVisualBP();
		if (VisualBP)
		{
			ItemVisual = GetWorld()->SpawnActor<AFactoryItemVisual>(
				VisualBP, Location, Rotation);
			
			if (!ItemVisual)
			{
				UE_LOG(LogTemp, Error, TEXT("Factory Item Visual Creation Failed"));
				return nullptr;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Factory Item Visual Creation Failed"));
			return nullptr;
		}
	}
	ItemVisual->SetActorLocationAndRotation(Location, Rotation);
	ItemVisual->UpdateVisual(ItemData);
	ItemVisual->SetActorHiddenInGame(false);
	
	return ItemVisual;
}

void UFactoryPoolSubsystem::ReturnItemVisualToPool(AFactoryItemVisual* ItemVisual)
{
	if (!ItemVisual) return;
	
	ItemVisual->SetActorHiddenInGame(true);
	InactiveItemVisuals.Push(ItemVisual);
}
