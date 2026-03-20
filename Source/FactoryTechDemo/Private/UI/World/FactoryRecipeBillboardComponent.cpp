// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/FactoryRecipeBillboardComponent.h"


UFactoryRecipeBillboardComponent::UFactoryRecipeBillboardComponent()
{

}


void UFactoryRecipeBillboardComponent::UpdateUIPlacement(float DeltaTime, const FVector& CameraLoc, const FVector& CameraForward, const FVector& OwnerLoc)
{
	FVector TargetLoc = OwnerLoc;
	TargetLoc.Z += BillboardZHeight;
	
	FRotator CameraRot = CameraForward.Rotation();
	FRotator TargetRot = FRotator(0.f, CameraRot.Yaw + 180.f, 0.f); // 카메라를 쳐다보도록 180도 뒤집음 (위젯 기준 정면)

	// 부드러운 이동 및 회전
	FVector NewLoc = FMath::VInterpTo(GetComponentLocation(), TargetLoc, DeltaTime, InterpolationSpeed);
	FRotator NewRot = FMath::RInterpTo(GetComponentRotation(), TargetRot, DeltaTime, InterpolationSpeed);

	SetWorldLocationAndRotation(NewLoc, NewRot);
}
