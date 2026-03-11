// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/Core/FFactorySlot.h"
#include "FactoryInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventorySlotUpdated, int32, SlotIndex, FFactorySlot, SlotData);

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
	/**
	 * 인벤토리 간 아이템 이동 요청
	 * @param SourceInventory - 아이템을 옮길 원본 인벤토리
	 * @param SourceSlotIndex - 원본 인벤토리에서 옮길 아이템이 있는 슬롯 인덱스
	 * @param TargetSlotIndex - 대상 슬롯 인덱스 (기본값 -1, 자동으로 빈 슬롯에 추가)
	 * @return 이동 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Factory|Inventory")
	bool RequestTransferItem(UFactoryInventoryComponent* SourceInventory, int32 SourceSlotIndex, int32 TargetSlotIndex = -1);
	
	// 아이템을 특정 슬롯에 추가. 추가 아이템 개수 반환
	UFUNCTION(BlueprintCallable, Category = "Factory|Inventory")
	int32 AddItemToTargetSlot(int32 SlotIndex, const UFactoryItemData* ItemData, int32 Amount);
	
	// 아이템을 인벤토리에 슬롯 지정없이 추가 (필드에서 아이템 획득 등)
	UFUNCTION(BlueprintCallable, Category = "Factory|Inventory")
	int32 AutoAddItem(const UFactoryItemData* ItemData, int32 Amount);
	
	// 아이템을 특정 슬롯에서 제거
	UFUNCTION(BlueprintCallable, Category = "Factory|Inventory")
	bool RemoveItemFromTargetSlot(int32 SlotIndex, int32 Amount);
	
	// 아이템을 인벤토리에서 슬롯 지정없이 제거
	UFUNCTION(BlueprintCallable, Category = "Factory|Inventory")
	bool AutoRemoveItem(const UFactoryItemData* ItemData, int32 Amount);
	
	// 특정 아이템의 인벤토리 내 총 개수를 반환
	UFUNCTION(BlueprintCallable, Category = "Factory|Inventory")
	int32 GetTotalItemAmount (const UFactoryItemData* ItemData) const;
	
	// 인벤토리 내 아이템 정렬
	UFUNCTION(BlueprintCallable, Category = "Factory|Inventory")
	void SortInventory();
	
	UPROPERTY(BlueprintAssignable, Category = "Factory|Inventory")
	FOnInventorySlotUpdated OnSlotUpdated;	// 인벤토리 슬롯이 업데이트될 때 호출
	
	int32 GetMaxItemSlotCount() const { return MaxItemSlotCount; }
	
protected:
	// 인벤토리의 슬롯 총 갯수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Inventory")
	int32 MaxItemSlotCount = 20;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Factory|Inventory")
	TArray<FFactorySlot> InventorySlots;
	
	void InitializeInventory();
};
