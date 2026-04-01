// Fill out your copyright notice in the Description page of Project Settings.


#include "Placement/Previews/FactoryBeltBridgePreview.h"

#include "Logistics/Belts/FactoryBelt.h"


AFactoryBeltBridgePreview::AFactoryBeltBridgePreview()
{
	PrimaryActorTick.bCanEverTick = false;
}

EOverlapValidityResult AFactoryBeltBridgePreview::UpdateOverlapValidity()
{
	TArray<AFactoryPlaceObjectBase*> OverlappedObjects = GetOverlappingPlaceObjects();
	
	if (OverlappedObjects.Num() == 0)
	{
		CurrentValidity = EOverlapValidityResult::Valid;
		return CurrentValidity;
	}
	
	CurrentValidity = EOverlapValidityResult::Replace;

	for (AFactoryPlaceObjectBase* PlaceObj : OverlappedObjects)
	{
		if (!PlaceObj->IsA<AFactoryBelt>())
		{
			// 벨트가 아닌 다른 구조물과 겹치면 에러!
			CurrentValidity = EOverlapValidityResult::Invalid;
			return CurrentValidity;
		}
	}

	return CurrentValidity;
}



