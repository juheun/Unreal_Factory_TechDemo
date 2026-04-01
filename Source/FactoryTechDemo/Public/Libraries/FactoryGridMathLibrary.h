// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FactoryGridMathLibrary.generated.h"

class UFactoryPortComponentBase;
class AFactoryLogisticsObjectBase;
enum class EBeltType : uint8;
/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryGridMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// 그리드 기초 연산
	UFUNCTION(BlueprintPure, Category = "Factory|GridMath")
	static FVector GridToWorld(const FIntPoint& GridLocation, float GridLength, const float Height = 0.0f);
	UFUNCTION(BlueprintPure, Category = "Factory|GridMath")
	static FIntPoint WorldToGrid(const FVector& WorldLocation, float GridLength);
	UFUNCTION(BlueprintPure, Category = "Factory|GridMath")
	static FVector CalculateSnappedLocation(
		const FVector& RawLocation, const FIntPoint& GridSize, float GridLength);
	
	// 공간 탐색
	UFUNCTION(BlueprintCallable, Category = "Factory|GridMath", meta = (WorldContext = "WorldContextObject"))
	static TArray<AFactoryLogisticsObjectBase*> GetFacilitiesInGridBox(
		const UObject* WorldContextObject, const FIntPoint& StartGrid, const FIntPoint& EndGrid, float GridLength);
	UFUNCTION(BlueprintCallable, Category = "Factory|GridMath", meta = (WorldContext = "WorldContextObject"))
	static AFactoryLogisticsObjectBase* GetFacilityAtGrid(const UObject* WorldContextObject, const FVector& GridLocation, float GridLength);
	
	// 포트 스캔
	UFUNCTION(BlueprintCallable, Category = "Factory|GridMath")
	static bool TryGetSmartSnapPortGrid(const FVector& PointingLocation, AActor* HitActor, bool bIsOutput, 
		FIntPoint& OutGrid, UFactoryPortComponentBase*& OutPort, float GridLength);
	UFUNCTION(BlueprintCallable, Category = "Factory|GridMath", meta = (WorldContext = "WorldContextObject"))
	static bool TryFindNearPortDirection(const UObject* WorldContextObject, const FVector& SearchCenter, bool bFindInputPort, FVector& OutDir, float GridLength);
	
	// 기타
	UFUNCTION(BlueprintPure, Category = "Factory|GridMath")
	static EBeltType DetermineBeltType(const FVector& StartDir, const FVector& EndDir);
};
