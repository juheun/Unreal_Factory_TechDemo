// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/Components/FactoryFacilityWorldUIComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Player/Input/FactoryTopViewPawn.h"


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
		
		CachedTopViewPawn = Controller->GetTopViewPawn();
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
	
	if (CachedCurrentViewMode == EFactoryViewModeType::TopView)
	{
		SetWidgetSpace(EWidgetSpace::Screen);
	}
	else
	{
		SetWidgetSpace(EWidgetSpace::World);
		if (UUserWidget* WidgetObj = GetUserWidgetObject())
		{
			WidgetObj->SetRenderScale(FVector2D(1.f, 1.f));
		}
	}
}

