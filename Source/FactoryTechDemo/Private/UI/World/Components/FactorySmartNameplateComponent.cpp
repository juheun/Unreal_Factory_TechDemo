// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/Components/FactorySmartNameplateComponent.h"

#include "Placement/FactoryObjectData.h"
#include "Placement/FactoryPlaceObjectBase.h"
#include "Core/FactoryDeveloperSettings.h"
#include "UI/World/Widgets/FactoryNameplateWidget.h"


UFactorySmartNameplateComponent::UFactorySmartNameplateComponent()
{
	
}

void UFactorySmartNameplateComponent::InitNameplate(const UFactoryObjectData* Data)
{
	if (!GetUserWidgetObject())
	{
		InitWidget(); // 위젯 강제 로드
	}
        
	if (UFactoryNameplateWidget* NameplateWidget = Cast<UFactoryNameplateWidget>(GetUserWidgetObject()))
	{
		NameplateWidget->SetFacilityName(Data->ObjectName);
	}
}

void UFactorySmartNameplateComponent::BeginPlay()
{
	Super::BeginPlay();
	
	float GridLength = 100.f;
	if (const UFactoryDeveloperSettings* Settings = GetDefault<UFactoryDeveloperSettings>())
	{
		GridLength = Settings->GetGridLength();
	}
	if (AFactoryPlaceObjectBase* PlaceObject = Cast<AFactoryPlaceObjectBase>(GetOwner()))
	{
		if (const UFactoryObjectData* ObjectData = PlaceObject->GetObjectData())
		{
			ExtentX = ObjectData->GridSize.X * GridLength * 0.5f;
			ExtentY = ObjectData->GridSize.Y * GridLength * 0.5f;
		}
	}
}

void UFactorySmartNameplateComponent::UpdateUIPlacement(float DeltaTime, const FVector& CameraLoc,
	const FVector& CameraForward, const FVector& OwnerLoc)
{
	Super::UpdateUIPlacement(DeltaTime, CameraLoc, CameraForward, OwnerLoc);
	
	FVector TargetLoc = OwnerLoc;
	TargetLoc.Z += FixedZHeight;
	
	if (CachedCurrentViewMode == EFactoryViewModeType::TopView)
	{
		SetWorldLocation(TargetLoc);
	}
	else
	{
		// 설비 4면 중 카메라와 가장 가까운 면 찾기
		AActor* OwnerActor = GetOwner();
		FVector OwnerForward = OwnerActor->GetActorForwardVector();
		FVector OwnerRight = OwnerActor->GetActorRightVector();
		
		FVector Faces[4] = { OwnerForward, -OwnerForward, OwnerRight, -OwnerRight };
		FVector DirToCamera2D = (CameraLoc - OwnerLoc).GetSafeNormal2D();
		
		int32 BestFaceIndex = 0;
		float MaxDot = -2.f;
		
		for (int32 i = 0; i < 4; i++)
		{
			float Dot = FVector::DotProduct(Faces[i], DirToCamera2D);
			if (Dot > MaxDot)
			{
				MaxDot = Dot;
				BestFaceIndex = i;
			}
		}
		
		// 목표 위치 설정
		float PushDistance = (BestFaceIndex == 0 || BestFaceIndex == 1) ? ExtentX : ExtentY;
		PushDistance = PushDistance + 1.f;	// Z-fight 방지 위해 값을 약간 더해줌 
		TargetLoc += Faces[BestFaceIndex] * PushDistance;

		// 해당 면에서 바깥쪽을 바라보도록 회전값 설정
		FRotator TargetRot = Faces[BestFaceIndex].Rotation();

		// 이동/회전 적용
		FVector NewLoc = FMath::VInterpTo(GetComponentLocation(), TargetLoc, DeltaTime, InterpolationSpeed);
		FRotator NewRot = FMath::RInterpTo(GetComponentRotation(), TargetRot, DeltaTime, InterpolationSpeed);

		SetWorldLocationAndRotation(NewLoc, NewRot);
	}
}

