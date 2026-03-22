// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "FactoryWorldUIActivatorComponent.generated.h"


enum class EFactoryViewModeType : uint8;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryWorldUIActivatorComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UFactoryWorldUIActivatorComponent();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnActivatorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnActivatorEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnViewModeChanged(EFactoryViewModeType NewViewMode);
	
	UPROPERTY(EditDefaultsOnly, Category="Factory|UIActivator")
	float NormalViewRadius = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category="Factory|UIActivator")
	float TopViewRadius = 8000.f; // 탑뷰에서는 화면을 덮을 만큼 거대하게 확장
};
