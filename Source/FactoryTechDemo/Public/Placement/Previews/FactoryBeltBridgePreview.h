// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FactoryPlacePreview.h"
#include "FactoryBeltBridgePreview.generated.h"

UCLASS()
class FACTORYTECHDEMO_API AFactoryBeltBridgePreview : public AFactoryPlacePreview
{
	GENERATED_BODY()

public:
	AFactoryBeltBridgePreview();
	
	virtual EOverlapValidityResult UpdateOverlapValidity() override; 
};
