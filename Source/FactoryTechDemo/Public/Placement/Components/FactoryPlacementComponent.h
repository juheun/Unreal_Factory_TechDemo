// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Logistics/FactoryLogisticsTypes.h"
#include "FactoryPlacementComponent.generated.h"

class UFactoryFacilitySelectionComponent;
struct FInputActionValue;
class UFactoryPortComponentBase;
class UFactoryItemData;
class UFactoryInputConfig;
class UFactoryPoolSubsystem;
class AFactoryLogisticsObjectBase;
struct FBeltPlacementData;
class AFactoryPlayerController;
class UFactoryObjectData;
class AFactoryPlacePreview;
class UFactoryBeltBuilderComponent;

UENUM()
enum class EPlacementMode : uint8
{
	None				UMETA(DisplayName = "None"),
	PlaceFromInventory	UMETA(DisplayName = "Place From Inventory"),
	Move				UMETA(DisplayName = "Move"),
	BeltPlace			UMETA(DisplayName = "Belt Place"),
	Retrieve			UMETA(DisplayName = "Retrieve"),
	MultipleControl		UMETA(DisplayName = "Multiple Control"),
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
	virtual void BeginPlay() override;
	
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
	UPROPERTY(VisibleAnywhere, Category = "Factory|Components")
	TObjectPtr<UFactoryBeltBuilderComponent> BeltBuilder;
	
	UPROPERTY(VisibleAnywhere, Category = "Factory|Components")
	TObjectPtr<UFactoryFacilitySelectionComponent> SelectionComponent;
	
private:
	///// 모드 진입
	void ProcessClickAction();		// 컨트롤러에서 클릭 눌렀을 때 호출
	void RotatePlacementPreview();
	void CancelPlaceObject();
	void ToggleBeltPlaceMode();
	void ToggleRetrieveMode();
	void EnterSingleMoveMode();
	void ToggleMultipleControlMode();
	/////
	
	///// 다중 제어 관련
	void OnMultipleControlAddStarted();
	void OnMultipleControlAddCompleted();
	void OnMultipleControlRemoveStarted();
	void OnMultipleControlRemoveCompleted();
	void UpdateDragSelectionBox();
	void OnMultipleControlMove();
	void OnMultipleControlRetrieve();
	
	// 델리게이트 수신 핸들러
	UFUNCTION()
	void HandleObjectSelected(AFactoryLogisticsObjectBase* SelectedObject);
	UFUNCTION()
	void HandleObjectDeselected(AFactoryLogisticsObjectBase* DeselectedObject);
	UFUNCTION()
	void HandleSelectionCleared();
	/////
	
	///// 내부 핵심 로직
	bool PlaceObject();
	void RenderBeltPreviews(const TArray<FBeltPlacementData>& BeltPlacementDatas);
	/////
	
	///// 프리뷰 및 피벗 관리 헬퍼
	void SetupSinglePreview(UFactoryObjectData* Data, EPlacementMode Mode);
	void ClearAllPreviews();
	void StartObjectPlaceMode();
	void SetAccuratePlacementPivotCenterAndGridSize();	// 프리뷰 객체의 전체 그리드 크기 계산
	/////
	
	///// 풀링 및 생성 유틸리티
	UFactoryPoolSubsystem* GetPool() const;
	AFactoryPlacePreview* CreateAndInitPreview(const UFactoryObjectData* Data, const FVector& Loc = FVector::ZeroVector, 
		const FRotator& Rot = FRotator::ZeroRotator, AFactoryLogisticsObjectBase* OriginalObject = nullptr) const;
	/////
	
	///// 공간 연산 및 알고리즘 헬퍼
	bool TryGetPointingGridLocation(FVector& OutResultVec) const;
	
	//////내부 변수
	EPlacementMode CurrentPlacementMode = EPlacementMode::None;
	
	UPROPERTY()
	TWeakObjectPtr<AFactoryPlayerController> CachedPlayerController;
	
	UPROPERTY()
	TArray<TObjectPtr<AFactoryPlacePreview>> ActivePreviews;
	
	UPROPERTY()
	TObjectPtr<AActor> PlaceObjectPivotActor; // 배치 시 기준이 되는 피벗 액터
	FIntPoint PlaceObjectPivotGridSize;	// 피벗 액터의 그리드 크기
	
	// 기본 변수
	float GridLength = 100.f;
	const float MaxBuildTraceDistance = 1500.f;
};
