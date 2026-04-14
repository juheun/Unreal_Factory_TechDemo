// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/Components/FactoryFacilityWorldUIComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Player/Input/FactoryTopViewPawn.h"


UFactoryFacilityWorldUIComponent::UFactoryFacilityWorldUIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	SetHiddenInGame(true);
	SetVisibility(false);
	SetWidgetSpace(EWidgetSpace::World);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void UFactoryFacilityWorldUIComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AFactoryPlayerController* Controller = Cast<AFactoryPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		CachedPC = Controller;
		CachedTopViewPawn = Controller->GetTopViewPawn();
	}
	CachedCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	
	bIsAwake = true;
	GoToSleep();
}


void UFactoryFacilityWorldUIComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                     FActorComponentTickFunction* ThisTickFunction)
{
	if (!bIsAwake) return;
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	APlayerCameraManager* CameraManager = CachedCameraManager.Get();
	AActor* OwnerActor = GetOwner();
	if (!CameraManager || !OwnerActor) return;
	
	FVector CameraLoc = CameraManager->GetCameraLocation();
	FVector CameraForward = CameraManager->GetCameraRotation().Vector();
	FVector OwnerLoc = OwnerActor->GetActorLocation();
	FVector DirToMachine = (OwnerLoc - CameraLoc).GetSafeNormal();
	
	// 공통 시야각 판별 로직
	if (FVector::DotProduct(CameraForward, DirToMachine) < CullingDotThreshold)
	{
		if (IsVisible())
		{
			SetHiddenInGame(true);
			SetVisibility(false);
		}
		return;
	}
	
	if (!IsVisible())
	{
		SetHiddenInGame(false);
		SetVisibility(true);
	}
	
	// 탑뷰일때 줌 레벨에 따른 크기 조정
	if (CachedCurrentViewMode == EFactoryViewModeType::TopView)
	{
		if (!CachedTopViewPawn.IsValid())	// 혹시 탑뷰폰 캐싱 안됐다면 재시도
		{
			if (AFactoryPlayerController* Controller = Cast<AFactoryPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
			{
				CachedTopViewPawn = Controller->GetTopViewPawn();
			}
		}
		
		if (AFactoryTopViewPawn* TopViewPawn = CachedTopViewPawn.Get())
		{
			float CurrentOrtho = TopViewPawn->GetCurrentOrthoWidth();
			float MinOrtho = TopViewPawn->GetMinOrthoWidth();
			float MaxOrtho = TopViewPawn->GetMaxOrthoWidth();
			
			float Alpha = FMath::Clamp((CurrentOrtho - MinOrtho) / (MaxOrtho - MinOrtho), 0.0f, 1.0f);
			float TargetScale = FMath::Lerp(MaxWidgetScale, MinWidgetScale, Alpha);
			if (UUserWidget* WidgetObj = GetUserWidgetObject())
			{
				WidgetObj->SetRenderScale(FVector2D(TargetScale, TargetScale));
			}
		}
	}
	
	// 컬링을 통과했다면 자식 클래스의 배치 로직 실행
	UpdateUIPlacement(DeltaTime, CameraLoc, CameraForward, OwnerLoc);
}

void UFactoryFacilityWorldUIComponent::WakeUp()
{
	if (bIsAwake) return;
	bIsAwake = true;
	
	if (AFactoryPlayerController* PC = CachedPC.Get())
	{
		CachedCurrentViewMode = PC->GetCurrentViewMode();
		EWidgetSpace TargetSpace = (CachedCurrentViewMode == EFactoryViewModeType::TopView) ? EWidgetSpace::Screen : EWidgetSpace::World;
       
		if (GetWidgetSpace() != TargetSpace)
		{
			SetWidgetSpace(TargetSpace);
			if (TargetSpace == EWidgetSpace::World)
			{
				if (UUserWidget* WidgetObj = GetUserWidgetObject())
					WidgetObj->SetRenderScale(FVector2D(1.f, 1.f));
			}
		}
	}

	SetVisibility(true);
	SetHiddenInGame(false);
	SetComponentTickEnabled(true);
}

void UFactoryFacilityWorldUIComponent::GoToSleep()
{
	if (!bIsAwake) return;
	bIsAwake = false;
	if (GetWidgetSpace() != EWidgetSpace::World)
	{
		SetWidgetSpace(EWidgetSpace::World);
	}
	
	if (UUserWidget* WidgetObj = GetUserWidgetObject())
	{
		WidgetObj->SetRenderScale(FVector2D(1.f, 1.f));
		WidgetObj->SetVisibility(ESlateVisibility::Collapsed);
	}
	SetVisibility(false);
	SetHiddenInGame(true);
	SetComponentTickEnabled(false);
}


