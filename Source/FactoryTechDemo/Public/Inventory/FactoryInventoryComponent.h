// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/FFactoryInventorySlot.h"
#include "FactoryInventoryComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFactoryInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// 아이템을 특정 슬롯에 추가. 추가 아이템 개수 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItemToTargetSlot(int32 SlotIndex, FName ItemID, int32 Amount);
	
	// 아이템을 인벤토리에 슬롯 지정없이 추가 (필드에서 아이템 획득 등)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AutoAddItem(FName ItemID, int32 Amount);
	
	// 아이템을 특정 슬롯에서 제거
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemFromTargetSlot(int32 SlotIndex, int32 Amount);
	
	// 아이템을 인벤토리에서 슬롯 지정없이 제거
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AutoRemoveItem(FName ItemID, int32 Amount);
	
	// 특정 아이템의 인벤토리 내 총 개수를 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetTotalItemAmount (FName ItemID) const;
	
	// 인벤토리 내 아이템 정렬
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SortInventory();
	
	// 인벤토리의 슬롯 총 갯수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 MaxItemSlotCount = 20;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TArray<FFactoryInventorySlot> InventorySlots;
	
	void InitializeInventory();
};
