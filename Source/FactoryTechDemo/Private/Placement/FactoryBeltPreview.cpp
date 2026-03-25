// Fill out your copyright notice in the Description page of Project Settings.


#include "Placement/FactoryBeltPreview.h"

#include "Logistics/FactoryBelt.h"
#include "Logistics/FactoryBeltBridge.h"


AFactoryBeltPreview::AFactoryBeltPreview()
{
	PrimaryActorTick.bCanEverTick = false;
}

EOverlapValidityResult AFactoryBeltPreview::UpdateOverlapValidity()
{
	TArray<AFactoryPlaceObjectBase*> OverlappedObjects = GetOverlappingPlaceObjects();
	
	if (OverlappedObjects.Num() == 0)
	{
		CurrentValidity = EOverlapValidityResult::Valid;
		return CurrentValidity;
	}
	
	CurrentValidity = EOverlapValidityResult::Replace;	// 벨트를 만나면 덮어쓰기가 기본
	
	for (AFactoryPlaceObjectBase* PlaceObj : OverlappedObjects)
	{
		if (PlaceObj->IsA<AFactoryBeltBridge>())
		{
			// 브릿지가 있는 칸이면 스킵
			CurrentValidity = EOverlapValidityResult::Skip;
		}
		else if (!PlaceObj->IsA<AFactoryBelt>())
		{
			// 벨트나 브릿지가 아닌 객체와 겹치면 설치 불가
			CurrentValidity = EOverlapValidityResult::Invalid;
			return CurrentValidity;
		}
	}

	return CurrentValidity;
}

FVector AFactoryBeltPreview::GetBeltExitDirection() const
{
	FVector ActorForwardDir = GetActorForwardVector();
	FVector OutPortDir = ActorForwardDir;
	
	if (BeltType == EBeltType::LeftTurn) OutPortDir = ActorForwardDir.RotateAngleAxis(-90.f, FVector::UpVector);
	else if (BeltType == EBeltType::RightTurn) OutPortDir = ActorForwardDir.RotateAngleAxis(90.f, FVector::UpVector);
	
	return OutPortDir.GetSafeNormal();
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
