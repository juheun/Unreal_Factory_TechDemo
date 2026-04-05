// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryStressTestGenerator.generated.h"

class UFactoryItemData;
class UFactoryObjectData;
class AFactoryPlaceObjectBase;

UCLASS()
class FACTORYTECHDEMO_API AFactoryStressTestGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	AFactoryStressTestGenerator();

	// 에디터의 디테일 패널에서 버튼으로 클릭할 수 있게 노출
	UFUNCTION(CallInEditor, Category = "Factory|Test")
	void GenerateStressTestEnvironment();

	UFUNCTION(CallInEditor, Category = "Factory|Test")
	void ClearEnvironment();

protected:
	UPROPERTY(EditAnywhere, Category = "Factory|Test")
	TObjectPtr<UFactoryObjectData> BeltData;

	// (옵션) 포트 데이터가 지정되면 양 끝에 스폰
	UPROPERTY(EditAnywhere, Category = "Factory|Test")
	TObjectPtr<UFactoryObjectData> OutputPortData;
	
	UPROPERTY(EditAnywhere, Category = "Factory|Test")
	TObjectPtr<UFactoryItemData> OutputPortItemData;

	UPROPERTY(EditAnywhere, Category = "Factory|Test")
	TObjectPtr<UFactoryObjectData> InputPortData;

	UPROPERTY(EditAnywhere, Category = "Factory|Test")
	float GridLength = 100.f;

	UPROPERTY(EditAnywhere, Category = "Factory|Test")
	int32 ChunkSize = 100;

	// 2 x 2 = 4개의 Chunk (200x200 크기, 약 4만 개)
	UPROPERTY(EditAnywhere, Category = "Factory|Test")
	int32 ChunkCountX = 2; 

	UPROPERTY(EditAnywhere, Category = "Factory|Test")
	int32 ChunkCountY = 2;

private:
	void GenerateChunk(int32 OffsetX, int32 OffsetY);

	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedActors;
};