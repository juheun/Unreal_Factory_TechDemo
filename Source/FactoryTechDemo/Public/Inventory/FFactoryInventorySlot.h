#pragma once

#include "CoreMinimal.h"
#include "FFactoryInventorySlot.generated.h"

USTRUCT(BlueprintType)
struct FFactoryInventorySlot
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite , Category = "Inventory")
	FName ItemID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite , Category = "Inventory")
	int32 Amount;
	
	static const int32 MaxCapacity = 50;
	
	FFactoryInventorySlot() : ItemID(NAME_None), Amount(0) {};
	FFactoryInventorySlot(FName ItemID, int32 Amount) : ItemID(ItemID), Amount(Amount) {};
	
	// 아이템이 없는 슬롯이거나, 아이템이 있지만 개수가 0인 경우를 모두 빈 슬롯으로 간주
	bool IsEmpty() const { return ItemID == NAME_None || Amount == 0;}
	
	// 슬롯이 가득 찼는지 여부를 판단하는 함수
	bool IsFull() const { return Amount >= MaxCapacity; }
	
	// 슬롯의 남은 공간을 계산하는 함수
	int32 GetAvailableSpace() const { return FMath::Max(0,MaxCapacity - Amount); }
	
	// 슬롯에서 아이템을 비우는 함수
	void Clear()
	{
		ItemID = NAME_None;
		Amount = 0;
	}
};
