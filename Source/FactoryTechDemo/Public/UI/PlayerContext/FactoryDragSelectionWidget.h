// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FactoryDragSelectionWidget.generated.h"

class UBorder;
/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryDragSelectionWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void StartDrag(const FVector& WorldLocation);
	void UpdateDrag(APlayerController* PC, const FVector2D& CurrentMousePos);
	void StopDrag();
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> SelectionBorder;
	
private:
	FVector WorldStartLocation;
	bool bIsDragging = false;
};
