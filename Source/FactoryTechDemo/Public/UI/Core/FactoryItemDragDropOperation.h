// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "FactoryItemDragDropOperation.generated.h"

enum class EFactorySlotType : uint8;
class UFactorySlotWidget;
class UFactoryItemData;

/**
 * 아이템을 드래그 할 때 드래그 정보
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DragDrop", meta=(ExposeOnSpawn=true))
	EFactorySlotType SourceType;	// 아이템을 드래그 시작한 슬롯의 위치
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DragDrop", meta=(ExposeOnSpawn=true))
	TObjectPtr<UFactorySlotWidget> SourceSlotWidget;	// 아이템을 드래그 시작한 슬롯 위젯의 참조
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DragDrop", meta=(ExposeOnSpawn=true))
	const UFactoryItemData* ItemData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DragDrop", meta=(ExposeOnSpawn=true))
	int32 DraggedAmount;
};
