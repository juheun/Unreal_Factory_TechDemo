// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/FactoryInteractable.h"
#include "FactoryInteractionPoint.generated.h"

class UWidgetComponent;
class UFactoryItemData;

UENUM(BlueprintType)
enum class EInteractionPointType : uint8
{
	GiveToInventory		UMETA(DisplayName="Give to Inventory"),
	TakeToInventory		UMETA(DisplayName="Take to Inventory"),
	GiveToWarehouse		UMETA(DisplayName="Give to Warehouse"),
	TakeToWarehouse		UMETA(DisplayName="Take to Warehouse"),
};

UCLASS()
class FACTORYTECHDEMO_API AFactoryInteractionPoint : public AActor, public IFactoryInteractable
{
	GENERATED_BODY()

public:
	AFactoryInteractionPoint();
	
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	
	// 사용자가 액터와 상호작용할 때 호출되는 함수
	virtual void Interact(const AActor* Interactor, const EPlacementMode CurrentMode, int32 OptionIndex = 0) override;
	virtual bool TryGetInteractionOptions(const EPlacementMode CurrentMode, TArray<FInteractionOption>& OutOptions) const override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Factory|InteractionPoint")
	void UpdateWidgetInfo(const FText& Text, UTexture2D* Icon);
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|InteractionPoint")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factory|InteractionPoint")
	TObjectPtr<UWidgetComponent> WidgetComponent;
	
	UPROPERTY(EditAnywhere, Category="Factory|Test")
	EInteractionPointType PointType = EInteractionPointType::GiveToInventory;
	
	UPROPERTY(EditAnywhere, Category="Factory|Test")
	TObjectPtr<const UFactoryItemData> ItemData;
	
	UPROPERTY(EditAnywhere, Category="Factory|Test")
	int32 Amount = 50;
	
	UPROPERTY(EditAnywhere, Category = "Factory|Test")
	FText InteractText;
};
