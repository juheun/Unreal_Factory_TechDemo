// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "FactoryArrowMeshComponent.generated.h"

/**
 * 
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryArrowMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UFactoryArrowMeshComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	FColor ArrowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	float ArrowSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	float ArrowLength;

	void SetArrowLength(float InLength);
	void SetArrowColor(FColor NewColor);
	float GetArrowLength() const { return ArrowLength; }
	
#if WITH_EDITOR
	// 에디터에서 변수 값을 수정할 때 호출되는 함수
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	
	
	private:
	void UpdateVisuals();
};
