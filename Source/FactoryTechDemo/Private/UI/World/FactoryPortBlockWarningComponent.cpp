// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/FactoryPortBlockWarningComponent.h"

UFactoryPortBlockWarningComponent::UFactoryPortBlockWarningComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetCastShadow(false);
	SetHiddenInGame(true);	// 기본적으로 처음에 숨기기
}

void UFactoryPortBlockWarningComponent::OnPortBlockedCallback(bool bIsBlocked)
{
	SetHiddenInGame(!bIsBlocked);
}
