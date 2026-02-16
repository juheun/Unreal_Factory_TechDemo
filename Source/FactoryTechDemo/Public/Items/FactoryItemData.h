// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FactoryItemData.generated.h"

class AFactoryItemVisual;

UENUM(BlueprintType)
enum class EFactoryItemCategory : uint8
{
	Resource UMETA(DisplayName = "Resource"),
	Consumable UMETA(DisplayName = "Consumable"),
	Facility UMETA(DisplayName = "Facility"),
};

USTRUCT(BlueprintType)
struct FFactoryItemInstance
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<const class UFactoryItemData> ItemData;	//이 아이템이 무엇인지에 대한 정보
	
	UPROPERTY(Transient, SkipSerialization)
	TObjectPtr<AFactoryItemVisual> VisualActor; //실제로 공간상의 움직임을 표현하기 위한 액터
	
	bool IsValid() const { return ItemData != nullptr; }
	
	FFactoryItemInstance() : ItemData(nullptr), VisualActor(nullptr) {}
	FFactoryItemInstance(const UFactoryItemData* InData) : ItemData(InData), VisualActor(nullptr) {}
};

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Identity")
	FText ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Identity")
	FName ItemID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Identity")
	TObjectPtr<UTexture2D> ItemICon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Identity")
	EFactoryItemCategory ItemCategory = EFactoryItemCategory::Resource;
};
