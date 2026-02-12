// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/FactoryItemVisual.h"
#include "Items/FactoryItemData.h"


// Sets default values
AFactoryItemVisual::AFactoryItemVisual()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AFactoryItemVisual::UpdateVisual(const UFactoryItemData* ItemData)
{
	if (!GetWorld() || !GetWorld()->IsGameWorld()) return;
	if (!ItemData || !ItemData->ItemICon) return;
	
	// TODO : 풀링 서브시스템 구축 후 삭제
	if (MeshComponent && !MeshComponent->GetStaticMesh())
	{
		FString MeshPath = FString::Printf(TEXT("/Game/PlacementData/Mesh/SM_%s"), *ItemData->ItemID.ToString());
        
		UStaticMesh* LoadedMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *MeshPath));
        
		if (LoadedMesh)
		{
			MeshComponent->SetStaticMesh(LoadedMesh);
		}
		else
		{
			// 못 찾았을 때 기본 큐브라도 로드 (경로는 /Engine/...)
			UStaticMesh* CubeMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("/Engine/BasicShapes/Cube")));
			MeshComponent->SetStaticMesh(CubeMesh);
		}
	}
	
	if (!DynamicMaterial)
	{
		DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(0);
	}
	
	if (DynamicMaterial)
	{
		DynamicMaterial->SetTextureParameterValue(FName("ItemIcon"), ItemData->ItemICon);
	}
}

