// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryPlaceObjectBase.generated.h"

class UFactoryObjectData;

/**
 * 오브젝트 배치에 관한 기능을 가진 클래스
 */
UCLASS()
class FACTORYTECHDEMO_API AFactoryPlaceObjectBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFactoryPlaceObjectBase();
	
	virtual void InitObject(const UFactoryObjectData* Data);
	virtual void Tick(float DeltaTime) override;
	
	UStaticMesh* GetStaticMesh() const {return MeshComponent->GetStaticMesh();};
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Data")
	TObjectPtr<const UFactoryObjectData> DataAsset;
	
	
};
