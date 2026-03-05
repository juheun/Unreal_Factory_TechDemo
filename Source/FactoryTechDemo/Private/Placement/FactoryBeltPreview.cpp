// Fill out your copyright notice in the Description page of Project Settings.


#include "Placement/FactoryBeltPreview.h"


AFactoryBeltPreview::AFactoryBeltPreview()
{
	PrimaryActorTick.bCanEverTick = false;
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
