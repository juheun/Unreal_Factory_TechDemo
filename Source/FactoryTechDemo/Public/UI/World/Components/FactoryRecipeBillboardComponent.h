// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryFacilityWorldUIComponent.h"
#include "FactoryRecipeBillboardComponent.generated.h"

class UFactoryRecipeData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FACTORYTECHDEMO_API UFactoryRecipeBillboardComponent : public UFactoryFacilityWorldUIComponent
{
	GENERATED_BODY()
	
public:
	UFactoryRecipeBillboardComponent();
	
	virtual void BeginPlay() override;
	virtual void WakeUp() override;
	
	UFUNCTION()
	void OnRecipeChangedCallback(const UFactoryRecipeData* NewRecipe);
	
	UFUNCTION()
	void OnItemChangedCallback(const UFactoryItemData* NewItemData);

protected:
	virtual void UpdateUIPlacement(float DeltaTime, const FVector& CameraLoc, const FVector& CameraForward, const FVector& OwnerLoc) override;
	void SetIcon(UTexture2D* Icon);

	void RefreshIconUI();
	bool bIsIconSet = false;

	UPROPERTY(EditDefaultsOnly, Category="Factory|Billboard")
	float DefaultBillboardZHeight = 250.f;
	UPROPERTY(EditDefaultsOnly, Category="Factory|Billboard")
	float BillboardZHeight = 100.f;
		
	UPROPERTY(EditDefaultsOnly, Category="Factory|Billboard")
	float TopViewScreenOffset = -100.f;
	
	float CachedZOffset;
};