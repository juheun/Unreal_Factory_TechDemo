// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "FactoryPortBlockWarningComponent.generated.h"

/**
 * 
 */
UCLASS()
class FACTORYTECHDEMO_API UFactoryPortBlockWarningComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UFactoryPortBlockWarningComponent();
	
	UFUNCTION()
	void OnPortBlockedCallback(bool bIsBlocked);
};
