// Fill out your copyright notice in the Description page of Project Settings.


#include "Placement/FactoryBeltBridgePreview.h"

#include "Logistics/FactoryBelt.h"


AFactoryBeltBridgePreview::AFactoryBeltBridgePreview()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AFactoryBeltBridgePreview::UpdateOverlapValidity()
{
	TArray<AFactoryPlaceObjectBase*> OverlappedObjects = GetOverlappingPlaceObjects();
	
	bool bIsOverlappingInvalidObject = false;
	for (AFactoryPlaceObjectBase* PlaceObj : OverlappedObjects)
	{
		if (!PlaceObj->IsA<AFactoryBelt>())		// 겹친 객체중 벨트가 아닌게 있는지 검사
		{
			bIsOverlappingInvalidObject = true;
			break;
		}
	}

	bIsPlacementValid = !bIsOverlappingInvalidObject;
	return bIsPlacementValid;
}



