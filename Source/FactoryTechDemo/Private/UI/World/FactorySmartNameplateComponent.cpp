// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/FactorySmartNameplateComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Placement/FactoryObjectData.h"
#include "Placement/FactoryPlaceObjectBase.h"
#include "Settings/FactoryDeveloperSettings.h"


UFactorySmartNameplateComponent::UFactorySmartNameplateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	SetHiddenInGame(true);
	SetWidgetSpace(EWidgetSpace::World);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	
	// 뷰모드에 따른 델리게이트 구독
	if (AFactoryPlayerController* Controller = Cast<AFactoryPlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		CachedCurrentViewMode = Controller->GetCurrentViewMode();
		Controller->OnViewModeChanged.AddDynamic(this, &UFactorySmartNameplateComponent::OnViewModeChanged);
		
		OnViewModeChanged(CachedCurrentViewMode);
	}
}

void UFactorySmartNameplateComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                    FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (!CameraManager) return;
	
	FVector CameraLoc = CameraManager->GetCameraLocation();
	FVector CameraForward = CameraManager->GetCameraRotation().Vector();
	
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	FVector OwnerLoc = OwnerActor->GetActorLocation();
	FVector DirToMachine = (OwnerLoc - CameraLoc).GetSafeNormal();
	
	// 카메라에서 벗어난 객체에 대해서는 이름표 표시하지 않음
	if (FVector::DotProduct(CameraForward, DirToMachine) < CullingDotThreshold)
	{
		if (!bHiddenInGame) SetHiddenInGame(true);
		return;
	}
	
	if (bHiddenInGame) SetHiddenInGame(false);
	
	FVector TargetLoc = OwnerLoc;
	TargetLoc.Z = FixedZHeight;
	
	if (CachedCurrentViewMode == EFactoryViewModeType::TopView)
	{
		SetWorldLocation(TargetLoc);
	}
	else
	{
		// 설비 4면 중 카메라와 가장 가까운 면 찾기
		FVector OwnerForward = OwnerActor->GetActorForwardVector();
		FVector OwnerRight = OwnerActor->GetActorRightVector();
		
		FVector Faces[4] = { OwnerForward, -OwnerForward, OwnerRight, -OwnerRight };
		FVector DirToCamera2D = (CameraLoc - OwnerLoc).GetSafeNormal2D();
		
		int32 BestFaceIndex = 0;
		float MaxDot = -1.f;
		
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
		TargetLoc += Faces[BestFaceIndex] * PushDistance;

		// 해당 면에서 바깥쪽을 바라보도록 회전값 설정
		FRotator TargetRot = Faces[BestFaceIndex].Rotation();

		// 이동/회전 적용
		FVector NewLoc = FMath::VInterpTo(GetComponentLocation(), TargetLoc, DeltaTime, InterpolationSpeed);
		FRotator NewRot = FMath::RInterpTo(GetComponentRotation(), TargetRot, DeltaTime, InterpolationSpeed);

		SetWorldLocationAndRotation(NewLoc, NewRot);
	}
}

void UFactorySmartNameplateComponent::WakeUp()
{
	if (bIsAwake) return;
	bIsAwake = true;
	// Tick 내부 계산에 따라 SetHiddenInGame 설정
	SetComponentTickEnabled(true);
}

void UFactorySmartNameplateComponent::GoToSleep()
{
	if (!bIsAwake) return;
	bIsAwake = false;
	SetHiddenInGame(true);
	SetComponentTickEnabled(false);
}

void UFactorySmartNameplateComponent::OnViewModeChanged(EFactoryViewModeType NewViewMode)
{
	CachedCurrentViewMode = NewViewMode;
	
	if (CachedCurrentViewMode == EFactoryViewModeType::TopView)
	{
		SetWidgetSpace(EWidgetSpace::Screen);
	}
	else
	{
		SetWidgetSpace(EWidgetSpace::World);
	}
}

