// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/FactoryFacilityWorldUIComponent.h"

#include "Kismet/GameplayStatics.h"


UFactoryFacilityWorldUIComponent::UFactoryFacilityWorldUIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	SetHiddenInGame(true);
	SetWidgetSpace(EWidgetSpace::World);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void UFactoryFacilityWorldUIComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AFactoryPlayerController* Controller = Cast<AFactoryPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		CachedCurrentViewMode = Controller->GetCurrentViewMode();
		Controller->OnViewModeChanged.AddDynamic(this, &UFactoryFacilityWorldUIComponent::OnViewModeChanged);
		OnViewModeChanged(CachedCurrentViewMode);
	}
}


void UFactoryFacilityWorldUIComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	AActor* OwnerActor = GetOwner();
	if (!CameraManager || !OwnerActor) return;
	
	FVector CameraLoc = CameraManager->GetCameraLocation();
	FVector CameraForward = CameraManager->GetCameraRotation().Vector();
	FVector OwnerLoc = OwnerActor->GetActorLocation();
	FVector DirToMachine = (OwnerLoc - CameraLoc).GetSafeNormal();
	
	// 공통 시야각 판별 로직
	if (FVector::DotProduct(CameraForward, DirToMachine) < CullingDotThreshold)
	{
		if (!bHiddenInGame) SetHiddenInGame(true);
		return;
	}
	
	if (bHiddenInGame) SetHiddenInGame(false);
	
	// 컬링을 통과했다면 자식 클래스의 배치 로직 실행
	UpdateUIPlacement(DeltaTime, CameraLoc, CameraForward, OwnerLoc);
}

void UFactoryFacilityWorldUIComponent::WakeUp()
{
	if (bIsAwake) return;
	bIsAwake = true;
	// Tick 내부 계산에 따라 SetHiddenInGame 설정
	SetComponentTickEnabled(true);
}

void UFactoryFacilityWorldUIComponent::GoToSleep()
{
	if (!bIsAwake) return;
	bIsAwake = false;
	SetHiddenInGame(true);
	SetComponentTickEnabled(false);
}

void UFactoryFacilityWorldUIComponent::OnViewModeChanged(EFactoryViewModeType NewViewMode)
{
	CachedCurrentViewMode = NewViewMode;
}

