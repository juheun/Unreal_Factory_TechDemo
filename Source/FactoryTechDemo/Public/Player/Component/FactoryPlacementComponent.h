// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Logistics/FactoryLogisticsTypes.h"
#include "FactoryPlacementComponent.generated.h"

class UFactoryItemData;
//전방선언
class UFactoryInputConfig;
class UFactoryPoolSubsystem;
class AFactoryLogisticsObjectBase;
struct FBeltPlacementData;
class AFactoryPlayerController;
class UFactoryObjectData;
class AFactoryPlacePreview;

UENUM()
enum class EPlacementMode : uint8
{
	None				UMETA(DisplayName = "None"),
	PlaceFromInventory	UMETA(DisplayName = "Place From Inventory"),
	Move				UMETA(DisplayName = "Move"),
	BeltPlace			UMETA(DisplayName = "Belt Place"),
	Retrieve			UMETA(DisplayName = "Retrieve"),
};

/**
 * 오브젝트 배치 관련 로직을 담당하는 컴포넌트
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectPlacedFromInventorySignature, const UFactoryFacilityItemData*, ItemData, int32, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlacementModeChangedSignature, EPlacementMode, NewMode);
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFactoryPlacementComponent();
	
	// 외부 호출 함수 모음
	void SetUpInputComponent(UEnhancedInputComponent* PlayerInputComp, const UFactoryInputConfig* InputConfig);
	void UpdatePreviewState();
	void SetPlaceFromDataPreview(UFactoryObjectData* Data);	// 단일 객체 배치 프리뷰 설정
	
	EPlacementMode GetCurrentPlaceMode() const { return CurrentPlacementMode; }

	UPROPERTY(BlueprintAssignable, Category = "Factory|Placement")
	FOnPlacementModeChangedSignature OnPlacementModeChanged;
	UPROPERTY(BlueprintAssignable, Category = "Factory|Placement")
	FOnObjectPlacedFromInventorySignature OnObjectPlacedFromInventorySignature;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Factory|Data")
	TObjectPtr<UFactoryObjectData> BeltData;
	
private:
	///// 모드 진입
	void ProcessClickAction();		// 컨트롤러에서 클릭 눌렀을 때 호출
	void RotatePlacementPreview();
	void CancelPlaceObject();
	void SetMoveObjectToPreviews();	// 이미 설치된 객체의 데이터를 기반으로 프리뷰 생성
	void ToggleBeltPlaceMode();
	void ToggleRetrieveMode();
	void TryEnterMoveMode();
	/////
	
	///// 객체 선택 제어
	void SelectObject(AFactoryLogisticsObjectBase* TargetObject);	// 객체 선택. 추후 다중 선택 및 선택된 객체 그룹 이동/회전/철거 기능 구현 예정
	void DeselectObject(AFactoryLogisticsObjectBase* TargetObject);	// 객체 선택 해제
	void ClearObject();
	/////
	
	///// 내부 핵심 로직
	bool PlaceObject();
	void HandleBeltPlacementClick();
	void BeltPlacePreviewUpdate(TArray<FBeltPlacementData> BeltPlacementDatas);
	/////
	
	///// 프리뷰 및 피벗 관리 헬퍼
	void ResetBeltGuidePreview();
	void SetupSinglePreview(UFactoryObjectData* Data, EPlacementMode Mode);
	void ClearAllPreviews();
	void StartObjectPlaceMode();
	void CalculatePlacementPivotCenterAndGridSize();	// 프리뷰 객체의 전체 그리드 크기 계산
	/////
	
	///// 풀링 및 생성 유틸리티
	UFactoryPoolSubsystem* GetPool() const;
	AFactoryPlacePreview* CreateAndInitPreview(const UFactoryObjectData* Data, 
		const FVector& Loc = FVector::ZeroVector, const FRotator& Rot = FRotator::ZeroRotator) const;
	/////
	
	///// 공간 연산 및 알고리즘 헬퍼
	bool TryGetPointingGridLocation(FVector& OutResultVec) const;
	// 지정된 위치에 벨트 배치가 가능한지 확인 및 설치가능 여부 반환
	bool TryGetBeltStartData(const FVector& PointingLocation, FIntPoint& OutStartGrid, FVector& OutStartDir) const;
	
	FVector CalculateSnappedLocation(FVector RawLocation, FIntPoint GridSize) const;
	FIntPoint WorldToGrid(const FVector& WorldLocation) const;
    FVector GridToWorld(const FIntPoint& GridLocation, const float Height = 0.0f) const;
	
	TArray<FBeltPlacementData> CalculateBeltPath(const FIntPoint& StartPoint, const FIntPoint& EndPoint, 
		const FVector& StartPointDir, bool bAlternativeRoute = false) const;
	EBeltType DetermineBeltType(const FVector& StartDir, const FVector& EndDir) const;
	
	
	//////내부 변수
	
	EPlacementMode CurrentPlacementMode = EPlacementMode::None;
	
	UPROPERTY()
	TWeakObjectPtr<AFactoryPlayerController> CachedPlayerController;
	
	// 오브젝트 다중 선택 후 이동 모드나 다중 철거 기능 구현을 위해 선택된 객체들을 저장
	UPROPERTY()
	TArray<TObjectPtr<AFactoryLogisticsObjectBase>> SelectedLogisticsObjectBases;
	// 현재 배치 프리뷰 액터들. 단일 배치 모드에서는 1개, 다중 배치 모드에서는 여러 개가 존재할 수 있음
	UPROPERTY()
	TArray<TObjectPtr<AFactoryPlacePreview>> ActivePreviews;
	
	UPROPERTY()
	TObjectPtr<AActor> PlaceObjectPivotActor; // 배치 시 기준이 되는 피벗 액터
	FIntPoint PlaceObjectPivotGridSize;	// 피벗 액터의 그리드 크기
	
	// 벨트 건설 상태 관련 변수
	FIntPoint BeltStartPoint;
	FVector BeltStartDir;
	bool bIsWaitingDetermineBeltEnd = false;
	
	float GridLength = 100.f;
	const float MaxBuildTraceDistance = 1500.f;
};
