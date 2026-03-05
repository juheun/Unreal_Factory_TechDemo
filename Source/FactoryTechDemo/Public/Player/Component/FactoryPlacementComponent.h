// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Logistics/FactoryLogisticsTypes.h"
#include "FactoryPlacementComponent.generated.h"

class AFactoryLogisticsObjectBase;
struct FBeltPlacementData;
class AFactoryPlayerController;
class UFactoryObjectData;
class AFactoryPlacePreview;

UENUM()
enum class EPlacementMode : uint8
{
	None UMETA(DisplayName = "None"),
	PlaceFromData UMETA(DisplayName = "Place From Data"),
	Move UMETA(DisplayName = "Move"),
	BeltPlace UMETA(DisplayName = "Belt Place"),
};

/**
 * 오브젝트 배치 관련 로직을 담당하는 컴포넌트
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFactoryPlacementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	void UpdatePreviewState();
	
	void ProcessPlacementAction();	// 컨트롤러에서 호출하는 액션 함수
	
	void SetPlaceFromDataPreview(UFactoryObjectData* Data);	// 단일 객체 배치 프리뷰 설정
	void SetMoveObjectToPreviews();	// 이미 설치된 객체의 데이터를 기반으로 프리뷰 생성
	void RotatePlacementPreview() const;
	void PlaceObject();
	void CancelPlaceObject();
	
	bool ToggleBeltPlaceMode();
	void HandleBeltPlacementClick();
	void BeltPlacePreviewUpdate(TArray<FBeltPlacementData> BeltPlacementDatas);
	void ResetBeltGuidePreview();
	
	void SelectObject(AFactoryLogisticsObjectBase* TargetObject);	// 객체 선택. 추후 다중 선택 및 선택된 객체 그룹 이동/회전/철거 기능 구현 예정
	void DeselectObject(AFactoryLogisticsObjectBase* TargetObject);	// 객체 선택 해제
	void ClearObject();
	
	EPlacementMode GetCurrentPlaceMode() const { return CurrentPlacementMode; }
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Factory")
	TObjectPtr<UFactoryObjectData> BeltData;
	
private:
	void ClearAllPreviews();
	void StartObjectPlaceMode();
	void CalculatePlacementPivotCenterAndGridSize();	// 프리뷰 객체의 전체 그리드 크기 계산
	bool TryGetPointingGridLocation(FVector& OutResultVec) const;
	FVector CalculateSnappedLocation(FVector RawLocation, FIntPoint GridSize) const;
	
	TArray<FBeltPlacementData> CalculateBeltPath(const FIntPoint& StartPoint, const FIntPoint& EndPoint, const FVector& StartPointDir) const;
	EBeltType DetermineBeltType(const FVector& StartDir, const FVector& EndDir) const;
	FIntPoint WorldToGrid(const FVector& WorldLocation) const;
    FVector GridToWorld(const FIntPoint& GridLocation, const float Height = 0.0f) const;
	// 지정된 위치에 벨트 배치가 가능한지 확인 및 설치가능 여부 반환
	bool TryGetBeltStartData(const FVector& PointingLocation, FIntPoint& OutStartGrid, FVector& OutStartDir) const;
	
	// 오브젝트 다중 선택 후 이동 모드나 다중 철거 기능 구현을 위해 선택된 객체들을 저장
	UPROPERTY()
	TArray<TObjectPtr<AFactoryLogisticsObjectBase>> SelectedLogisticsObjectBases;
	// 현재 배치 프리뷰 액터들. 단일 배치 모드에서는 1개, 다중 배치 모드에서는 여러 개가 존재할 수 있음
	UPROPERTY()
	TArray<TObjectPtr<AFactoryPlacePreview>> ActivePreviews;
	UPROPERTY()
	TObjectPtr<AActor> PlaceObjectPivotActor; // 배치 시 기준이 되는 피벗 액터
	FIntPoint PlaceObjectPivotGridSize;	// 피벗 액터의 그리드 크기
	
	
	FIntPoint BeltStartPoint;
	FVector BeltStartDir;
	bool bIsWaitingDetermineBeltEnd = false;
	
	
	EPlacementMode CurrentPlacementMode = EPlacementMode::None;
	float GridLength = 100.f;
	const float MaxBuildTraceDistance = 1500.f;
	
	UPROPERTY()
	TWeakObjectPtr<AFactoryPlayerController> CachedPlayerController;
};
