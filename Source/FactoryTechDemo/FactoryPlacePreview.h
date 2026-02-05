// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryPlacePreview.generated.h"

class UFactoryObjectData;
class UBoxComponent;
class UDecalComponent;

UCLASS()
class FACTORYTECHDEMO_API AFactoryPlacePreview : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFactoryPlacePreview();
	
	void InitPreview(const UFactoryObjectData* Data); // 데이터 에셋을 받아 메쉬, 데칼 크기 등 설정
	bool GetPlacementValid() const { return bIsPlacementValid; }
	const UFactoryObjectData* GetObjectData() const { return ObjectData.Get(); } 
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	UPROPERTY(VisibleAnywhere, Category = "Visual")
	TObjectPtr<UDecalComponent> GridDecalComponent;
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	TObjectPtr<UBoxComponent> OverlapBox;
	
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewDynamicMaterial;	// 배치불가시 색상변경 위함
	void SetPlacementValid(bool bIsValid);	// 배치가능 여부에 따라 색상 변경
	bool bIsPlacementValid = true;
	
	UPROPERTY()
	TObjectPtr<const UFactoryObjectData> ObjectData;
};
