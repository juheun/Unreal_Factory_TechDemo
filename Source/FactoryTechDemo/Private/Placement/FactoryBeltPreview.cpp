// Fill out your copyright notice in the Description page of Project Settings.


#include "Placement/FactoryBeltPreview.h"

#include "Logistics/FactoryBelt.h"


AFactoryBeltPreview::AFactoryBeltPreview()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AFactoryBeltPreview::UpdateOverlapValidity()
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

void AFactoryBeltPreview::SetBeltType(const EBeltType Type)
{
	BeltType = Type;
	
	if (!MeshComponent) return;
	TObjectPtr<UStaticMesh>* FoundMesh = BeltMeshMap.Find(Type);
	
	if (FoundMesh && *FoundMesh)
	{
		MeshComponent->SetStaticMesh(BeltMeshMap[Type]);
	}
}
