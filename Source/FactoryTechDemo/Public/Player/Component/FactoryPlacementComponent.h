// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	FirstPlace UMETA(DisplayName = "First Place"),
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
	
	void SetFirstPlacePreview(UFactoryObjectData* Data);	// 단일 객체 배치 프리뷰 설정
	void SetMoveObjectToPreviews();	// 이미 설치된 객체의 데이터를 기반으로 프리뷰 생성
	//void StartBeltPlaceMode();
	void RotatePlacementPreview() const;
	void PlaceObject();
	void CancelPlaceObject();
	
	void SelectObject(AFactoryLogisticsObjectBase* TargetObject);	// 객체 선택. 추후 다중 선택 및 선택된 객체 그룹 이동/회전/철거 기능 구현 예정
	void DeselectObject(AFactoryLogisticsObjectBase* TargetObject);	// 객체 선택 해제
	void ClearObject();
	
	bool GetIsPlaceMode() const { return CurrentPlacementMode != EPlacementMode::None; }
	
private:
	void ClearAllPreviews();
	void StartObjectPlaceMode();
	void CalculatePlacementPivotCenterAndGridSize();	// 프리뷰 객체의 전체 그리드 크기 계산
	FVector GetPlacementObjectLocation() const;
	FVector CalculateSnappedLocation(FVector RawLocation, FIntPoint GridSize) const;
	
	//TODO : 추후 밸트 연속 배치 구현
	//TArray<FBeltPlacementData> CalculateBeltPath(FIntPoint StartPoint, FIntPoint EndPoint) const;
	
	// 오브젝트 다중 선택 후 이동 모드나 다중 철거 기능 구현을 위해 선택된 객체들을 저장
	UPROPERTY()
	TArray<TObjectPtr<AFactoryLogisticsObjectBase>> SelectedLogisticsObjectBases;
	// 현재 배치 프리뷰 액터들. 단일 배치 모드에서는 1개, 다중 배치 모드에서는 여러 개가 존재할 수 있음
	UPROPERTY()
	TArray<TObjectPtr<AFactoryPlacePreview>> ActivePreviews;
	UPROPERTY()
	TObjectPtr<AActor> PlaceObjectPivotActor; // 배치 시 기준이 되는 피벗 액터
	FIntPoint PlaceObjectPivotGridSize;	// 피벗 액터의 그리드 크기
	
	EPlacementMode CurrentPlacementMode = EPlacementMode::None;
	float GridLength = 100.f;
	const float MaxBuildTraceDistance = 1500.f;
	
	UPROPERTY()
	TWeakObjectPtr<AFactoryPlayerController> CachedPlayerController;
};
