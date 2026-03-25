// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FactoryPlacePreview.generated.h"

class UFactoryObjectData;
class UBoxComponent;
class UDecalComponent;
class AFactoryPlaceObjectBase;

UENUM(BlueprintType)
enum class EOverlapValidityResult : uint8
{
	Valid,      // 설치 가능
	Invalid,    // 설치 불가
	Skip,       // 모종의 이유로 설치를 스킵 (프리뷰 투명화)
	Replace     // 겹친 건물을 부수고 내가 덮어씀
};

UCLASS()
class FACTORYTECHDEMO_API AFactoryPlacePreview : public AActor
{
	GENERATED_BODY()
	
public:	
	AFactoryPlacePreview();
	
	void InitPreview(const UFactoryObjectData* Data); // 데이터 에셋을 받아 메쉬, 데칼 크기 등 설정
	
	virtual EOverlapValidityResult UpdateOverlapValidity();	// 현재 다른 물체와 겹쳐있는지 여부 검사
	void SetVisualValidity(bool const bIsValid);	// 배치가능 여부에 따라 색상 변경
	
	TArray<AFactoryPlaceObjectBase*> GetOverlappingPlaceObjects() const;	// 겹쳐있는 객체들중 유효한 객체만 필터링해서 반환
	
	EOverlapValidityResult GetPlacementValid() const { return CurrentValidity; }
	const UFactoryObjectData* GetObjectData() const { return ObjectData.Get(); }
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	UPROPERTY(VisibleAnywhere, Category = "Visual")
	TObjectPtr<UDecalComponent> GridDecalComponent;
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	TObjectPtr<UBoxComponent> OverlapBox;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewDynamicMaterial;	// 배치불가시 색상변경 위함
	EOverlapValidityResult CurrentValidity = EOverlapValidityResult::Valid;
	
	UPROPERTY()
	TObjectPtr<const UFactoryObjectData> ObjectData;
};
