// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/Components/UFactoryFacilityBlockWarningComponent.h"

#include "Kismet/GameplayStatics.h"


UFactoryFacilityBlockWarningComponent::UFactoryFacilityBlockWarningComponent()
{
	SetDrawSize(FVector2D(50.f, 50.f));
	SetHiddenInGame(true);
}

void UFactoryFacilityBlockWarningComponent::BeginPlay()
{
	Super::BeginPlay();
	SetDrawSize(FVector2D(50.f, 50.f));
}

void UFactoryFacilityBlockWarningComponent::OnFacilityBlockCallback(bool bIsBlocked)
{
	SetDrawSize(FVector2D(50.f, 50.f));
	SetHiddenInGame(!bIsBlocked);
	
	if (!GetUserWidgetObject())
	{
		InitWidget();
	}
	
	if (UUserWidget* WidgetObj = GetUserWidgetObject())
	{
		WidgetObj->SetVisibility(bIsBlocked ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UFactoryFacilityBlockWarningComponent::UpdateUIPlacement(float DeltaTime, const FVector& CameraLoc,
                                                              const FVector& CameraForward, const FVector& OwnerLoc)
{
	Super::UpdateUIPlacement(DeltaTime, CameraLoc, CameraForward, OwnerLoc);
	
	FVector TargetLoc = OwnerLoc;
	
	if (AActor* OwnerActor = GetOwner())
	{
		FBox MeshBounds(ForceInit);
		
		TArray<UStaticMeshComponent*> MeshComps;
		OwnerActor->GetComponents<UStaticMeshComponent>(MeshComps);
		for (UStaticMeshComponent* Mesh : MeshComps)
		{
			MeshBounds += Mesh->Bounds.GetBox();
		}
		
		if (MeshBounds.IsValid)
		{
			TargetLoc.Z = MeshBounds.Max.Z + BillboardZHeight;
		}
		else
		{
			TargetLoc.Z += DefaultBillboardZHeight;
		}
	}
	else
	{
		TargetLoc.Z += DefaultBillboardZHeight; // 예외 처리 (기본값)
	}
	
	if (CachedCurrentViewMode == EFactoryViewModeType::TopView)
	{
		if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			FVector RealCameraUp = CameraManager->GetCameraRotation().Quaternion().GetUpVector();
			TargetLoc += RealCameraUp * TopViewScreenOffset;
		}
	}
    
	// 카메라 보게하기
	FRotator CameraRot = CameraForward.Rotation();
	FRotator TargetRot = FRotator(0.f, CameraRot.Yaw + 180.f, 0.f);

	SetWorldLocationAndRotation(TargetLoc, TargetRot);
}
