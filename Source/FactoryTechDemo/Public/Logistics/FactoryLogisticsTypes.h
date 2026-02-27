#pragma once

#include "CoreMinimal.h"
#include "FactoryLogisticsTypes.generated.h"

UENUM(BlueprintType)
enum class EBeltType : uint8
{
	Straight UMETA(DisplayName = "Straight"), 
	LeftTurn UMETA(DisplayName = "LeftTurn"),
	RightTurn UMETA(DisplayName = "RightTurn")
};

// 배치 미리보기 및 실제 배치를 위한 데이터 팩
USTRUCT(BlueprintType)
struct FBeltPlacementData
{
	GENERATED_BODY()

	UPROPERTY()
	FIntPoint GridLocation; // 그리드 좌표 (X, Y)

	UPROPERTY()
	FRotator Rotation; // 배치될 회전값

	UPROPERTY()
	EBeltType Type; // 직선/곡선 여부

	FBeltPlacementData() 
		: GridLocation(FIntPoint::ZeroValue), Rotation(FRotator::ZeroRotator), Type(EBeltType::Straight) {}
};