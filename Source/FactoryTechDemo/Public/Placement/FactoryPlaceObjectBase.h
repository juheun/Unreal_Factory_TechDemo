// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/FactoryInteractable.h"
#include "UI/Storage/FactoryStorageMenuWidget.h"
#include "FactoryPlaceObjectBase.generated.h"

class UFactoryObjectData;

/**
 * 오브젝트 배치에 관한 기능을 가진 클래스
 */
UCLASS()
class FACTORYTECHDEMO_API AFactoryPlaceObjectBase : public AActor, public IFactoryInteractable
{
	GENERATED_BODY()

public:
	AFactoryPlaceObjectBase();
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void InitObject(const UFactoryObjectData* Data);
	virtual void Tick(float DeltaTime) override;
	
	virtual void Interact(const AActor* Interactor, const EPlacementMode CurrentMode) override;
	virtual bool TryGetInteractText(const EPlacementMode CurrentMode, FText& OutText) const override;
	void Retrieve();
	
	UStaticMesh* GetStaticMesh() const {return MeshComponent->GetStaticMesh();};

	TObjectPtr<const UFactoryObjectData> GetObjectData() const {return PlacementDataAsset;}
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Data")
	TObjectPtr<const UFactoryObjectData> PlacementDataAsset;
	
	UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
	EFactoryMenuMode FacilityMenuMode = EFactoryMenuMode::Facility;
};
