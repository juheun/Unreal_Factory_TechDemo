// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/FactoryPortBlockWarningComponent.h"

#include "Settings/FactoryDeveloperSettings.h"

UFactoryPortBlockWarningComponent::UFactoryPortBlockWarningComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetCastShadow(false);
	SetHiddenInGame(true);	// 기본적으로 처음에 숨기기
	
	SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
}

void UFactoryPortBlockWarningComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (const UFactoryDeveloperSettings* Settings = GetDefault<UFactoryDeveloperSettings>())
	{
		if (UStaticMesh* WarningMesh = Settings->GetPortWarningMesh())
		{
			SetStaticMesh(WarningMesh);
		}
	}
}

void UFactoryPortBlockWarningComponent::OnPortBlockedCallback(bool bIsBlocked)
{
	SetHiddenInGame(!bIsBlocked);
}
