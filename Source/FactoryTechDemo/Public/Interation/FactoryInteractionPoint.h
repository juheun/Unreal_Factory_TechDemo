// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/FactoryInteractable.h"
#include "FactoryInteractionPoint.generated.h"

class UFactoryItemData;

UCLASS()
class FACTORYTECHDEMO_API AFactoryInteractionPoint : public AActor, public IFactoryInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFactoryInteractionPoint();
	
	// 사용자가 액터와 상호작용할 때 호출되는 함수
	virtual void Interact(AActor* Interactor) override;
	virtual FText GetInteractText() const override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Factory|InteractionPoint")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditAnywhere, Category="Factory|Test")
	TObjectPtr<UFactoryItemData> ItemToGive;
	
	UPROPERTY(EditAnywhere, Category="Factory|Test")
	int32 AmountToGive = 50;
	
	UPROPERTY(EditAnywhere, Category = "Factory|Test")
	FText InteractText = FText();
};
