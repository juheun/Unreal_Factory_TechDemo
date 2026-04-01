// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactoryBeltBuilderComponent.generated.h"


struct FBeltPlacementData;
class UFactoryPortComponentBase;
class UFactoryObjectData;
class UFactoryPlacementComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryBeltBuilderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFactoryBeltBuilderComponent();
	
	void ResetBuilderState();
	
	// 마우스 위치를 받아 벨트 프리뷰 경로 계산 및 업데이트
	bool GetPreviewPathData(
		const FVector& PointingLocation, float GridLength, bool bAlternativeRoute, TArray<FBeltPlacementData>& OutPathData) const;
	FVector GetSnappedStartLocation(const FVector& PointingLocation, float GridLength) const;
	// 마우스 클릭 시 호출. 시작점을 찍었는지, 아니면 종료점을 찍어 실제 스폰을 해야 하는지 결정
	bool ProcessClick(const FVector& PointingLocation, float GridLength);
	
	bool IsWaitingForEndPoint() const { return bIsWaitingDetermineBeltEnd; }
	UFactoryObjectData* GetBeltData() const { return BeltData; }
	UFactoryObjectData* GetBeltBridgeData() const { return BeltBridgeData; }
	FVector GetBeltStartDir() const { return BeltStartDir; }
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Factory|Data")
	TObjectPtr<UFactoryObjectData> BeltData;
	
	UPROPERTY(EditDefaultsOnly, Category = "Factory|Data")
	TObjectPtr<UFactoryObjectData> BeltBridgeData;
	
private:
	TArray<FBeltPlacementData> CalculateBeltPath(const FIntPoint& StartPoint, const FIntPoint& EndPoint, 
		const FVector& StartPointDir, float GridLength, bool bAlternativeRoute = false, UFactoryPortComponentBase* TargetPort = nullptr) const;
	
	bool TryGetBeltStartData(const FVector& PointingLocation, float GridLength, FIntPoint& OutStartGrid, FVector& OutStartDir) const;
	
	FIntPoint BeltStartPoint;
	FVector BeltStartDir;
	bool bIsWaitingDetermineBeltEnd = false;
};
