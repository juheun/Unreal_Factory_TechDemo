// Fill out your copyright notice in the Description page of Project Settings.

#include "Test/FactoryStressTestGenerator.h"
#include "Libraries/FactoryGridMathLibrary.h"
#include "Logistics/Belts/FactoryBelt.h"
#include "Placement/FactoryObjectData.h"
#include "Placement/FactoryPlaceObjectBase.h"
#include "Engine/World.h"
#include "Logistics/Warehouse/FactoryWarehouseExporter.h"
#include "Logistics/Warehouse/FactoryWarehouseImporter.h"

AFactoryStressTestGenerator::AFactoryStressTestGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFactoryStressTestGenerator::GenerateStressTestEnvironment()
{
	// 겹쳐서 소환되는 것을 방지하기 위해 기존 테스트 환경 먼저 초기화
	ClearEnvironment(); 

	if (!BeltData)
	{
		UE_LOG(LogTemp, Warning, TEXT("StressTestGenerator: BeltData is missing!"));
		return;
	}

	// 지정된 Chunk 갯수만큼 100x100 단위의 지그재그 벨트망 생성
	for (int32 cx = 0; cx < ChunkCountX; ++cx)
	{
		for (int32 cy = 0; cy < ChunkCountY; ++cy)
		{
			GenerateChunk(cx * ChunkSize, cy * ChunkSize);
		}
	}
}

void AFactoryStressTestGenerator::ClearEnvironment()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Empty();
}

void AFactoryStressTestGenerator::GenerateChunk(int32 OffsetX, int32 OffsetY)
{
	TArray<FIntPoint> PathPoints;
	
	FName ChunkFolderPath = FName(*FString::Printf(TEXT("StressTest/Chunk_%d_%d"), OffsetX / ChunkSize, OffsetY / ChunkSize));

	// 포트를 위해 양 끝단 1칸씩(또는 설정에 따라) 비워둠
	// 예: ChunkSize가 100이면 1 ~ 98 까지만 순회
	int32 MinX = OffsetX + 1;
	int32 MaxX = OffsetX + ChunkSize - 2;
	int32 MinY = OffsetY + 1;
	int32 MaxY = OffsetY + ChunkSize - 2;

	// 1. 뱀 모양(Serpentine) 그리드 좌표 계산
	for (int32 y = MinY; y <= MaxY; ++y)
	{
		int32 LocalY = y - OffsetY;
		
		if (LocalY % 2 != 0) // 홀수 줄: 왼쪽에서 오른쪽으로 (+X)
		{
			for (int32 x = MinX; x <= MaxX; ++x) PathPoints.Add(FIntPoint(x, y));
		}
		else // 짝수 줄: 오른쪽에서 왼쪽으로 (-X)
		{
			for (int32 x = MaxX; x >= MinX; --x) PathPoints.Add(FIntPoint(x, y));
		}
	}

	// 2. 계산된 좌표를 바탕으로 벨트 타입 결정 및 스폰
	for (int32 i = 0; i < PathPoints.Num(); ++i)
	{
		// 시작과 끝 방향 벡터 연산
		FVector InDir = (i == 0) ? FVector(1, 0, 0) : FVector(PathPoints[i] - PathPoints[i - 1], 0.f).GetSafeNormal();
		FVector EndDir = (i == PathPoints.Num() - 1) ? InDir : FVector(PathPoints[i + 1] - PathPoints[i], 0.f).GetSafeNormal();

		EBeltType Type = UFactoryGridMathLibrary::DetermineBeltType(InDir, EndDir);
		FRotator Rot = InDir.Rotation();
		FVector SpawnLoc = UFactoryGridMathLibrary::GridToWorld(PathPoints[i], GridLength);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		AFactoryPlaceObjectBase* NewBelt = GetWorld()->SpawnActor<AFactoryPlaceObjectBase>(
			BeltData->PlaceObjectBP, SpawnLoc, Rot, Params);

		if (NewBelt)
		{
			NewBelt->SetFolderPath(ChunkFolderPath);
			
			NewBelt->InitObject(BeltData);
			if (AFactoryBelt* Belt = Cast<AFactoryBelt>(NewBelt))
			{
				Belt->SetBeltType(Type);
			}
			SpawnedActors.Add(NewBelt);
		}
	}

	// 3. 포트 스폰 (옵션)
	if (OutputPortData)
	{
		// 청크 시작점 (홀수 줄의 시작)
		FVector PortLoc = UFactoryGridMathLibrary::GridToWorld(FIntPoint(MinX - 1, MinY), GridLength);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AFactoryWarehouseExporter* WarehouseExporter = GetWorld()->SpawnActor<AFactoryWarehouseExporter>(OutputPortData->PlaceObjectBP, PortLoc, FRotator::ZeroRotator, Params);
		if (WarehouseExporter)
		{
			WarehouseExporter->SetFolderPath(ChunkFolderPath);
			
			WarehouseExporter->InitObject(OutputPortData);
			SpawnedActors.Add(WarehouseExporter);
			WarehouseExporter->SetTargetItem(OutputPortItemData);
		}
	}

	if (InputPortData)
	{
		// 청크 종료점 (마지막 줄의 진행 방향 끝)
		FIntPoint LastPoint = PathPoints.Last();
		bool bEndsRight = ((MaxY - OffsetY) % 2 != 0);
		FIntPoint EndPortGrid = bEndsRight ? FIntPoint(LastPoint.X + 1, LastPoint.Y) : FIntPoint(LastPoint.X - 1, LastPoint.Y);
		FRotator EndRot = bEndsRight ? FRotator(0, 0, 0) : FRotator(0, 180, 0);

		FVector PortLoc = UFactoryGridMathLibrary::GridToWorld(EndPortGrid, GridLength);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AFactoryWarehouseImporter* WarehouseImporter = GetWorld()->SpawnActor<AFactoryWarehouseImporter>(InputPortData->PlaceObjectBP, PortLoc, EndRot, Params);
		if (WarehouseImporter)
		{
			WarehouseImporter->SetFolderPath(ChunkFolderPath);
			
			WarehouseImporter->InitObject(InputPortData);
			SpawnedActors.Add(WarehouseImporter);
		}
	}
}