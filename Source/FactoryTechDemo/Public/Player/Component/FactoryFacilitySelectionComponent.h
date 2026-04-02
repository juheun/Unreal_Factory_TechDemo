// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactoryFacilitySelectionComponent.generated.h"


class UFactoryDragSelectionWidget;
class AFactoryLogisticsObjectBase;
class AFactoryPlayerController;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectSelected, AFactoryLogisticsObjectBase*, SelectedObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectDeselected, AFactoryLogisticsObjectBase*, DeselectedObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectionCleared);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryFacilitySelectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFactoryFacilitySelectionComponent();
	
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintAssignable, Category="Factory|Selection|Event")
	FOnObjectSelected OnObjectSelected;
	UPROPERTY(BlueprintAssignable, Category="Factory|Selection|Event")
	FOnObjectDeselected OnObjectDeselected;
	UPROPERTY(BlueprintAssignable, Category="Factory|Selection|Event")
	FOnSelectionCleared OnSelectionCleared;
	
	void BeginDragSelection(const FVector& HitLocation, float GridLength, bool bIsRemove);
	void UpdateDragSelectionBox(const FVector& HitLocation, float GridLength);
	// 반환값이 true면 단순 클릭으로 판정됨을 알림
	bool EndDragSelection(const FVector& HitLocation, float GridLength);
	
	void SelectObject(AFactoryLogisticsObjectBase* TargetObject);	// 객체 선택. 추후 다중 선택 및 선택된 객체 그룹 이동/회전/철거 기능 구현 예정
	void DeselectObject(AFactoryLogisticsObjectBase* TargetObject);	// 객체 선택 해제
	void ClearSelectedObjects();
	void SelectConnectedBeltLine(const FVector& HitLocation, float GridLength);
	
	const TArray<TObjectPtr<AFactoryLogisticsObjectBase>>& GetSelectedObjects() const { return SelectedObjects; }
	
protected:
	void InitializeSelectionUI();
	
	UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
	TSubclassOf<UFactoryDragSelectionWidget> DragSelectionWidgetBP;
	UPROPERTY(VisibleAnywhere, Category = "Factory|UI")
	TObjectPtr<UFactoryDragSelectionWidget> DragSelectionWidget;
	
private:
	UPROPERTY()
	TWeakObjectPtr<AFactoryPlayerController> CachedPlayerController;
	
	// 오브젝트 다중 선택 후 이동 모드나 다중 철거 기능 구현을 위해 선택된 객체들을 저장
	UPROPERTY()
	TArray<TObjectPtr<AFactoryLogisticsObjectBase>> SelectedObjects;
	UPROPERTY()
	TSet<TObjectPtr<AFactoryLogisticsObjectBase>> PreDragSelection;
	
	//다중제어 관련 변수
	bool bIsDraggingSelection = false;
	bool bIsRemoveDrag = false;
	FIntPoint SelectionDragStartPoint;
};
