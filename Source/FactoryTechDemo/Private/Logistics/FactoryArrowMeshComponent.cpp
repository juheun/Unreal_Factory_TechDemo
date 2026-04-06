// Fill out your copyright notice in the Description page of Project Settings.

#include "Logistics/FactoryArrowMeshComponent.h"
#include "Core/FactoryDeveloperSettings.h"
#include "Materials/MaterialInstanceDynamic.h"

UFactoryArrowMeshComponent::UFactoryArrowMeshComponent()
{
	// 기본값 세팅
	ArrowColor = FColor::Red;
	ArrowSize = 1.0f;
	ArrowLength = 100.0f;
	
	SetCollisionEnabled(ECollisionEnabled::NoCollision); // 화살표는 충돌이 필요 없음
	SetCastShadow(false); // 렌더링 최적화
}

#if WITH_EDITOR
void UFactoryArrowMeshComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	UpdateVisuals();
}
#endif

void UFactoryArrowMeshComponent::UpdateVisuals()
{
	if (HasAnyFlags(RF_ClassDefaultObject)) return;
	
	if (const UFactoryDeveloperSettings* Settings = GetDefault<UFactoryDeveloperSettings>())
	{
		if (UStaticMesh* ArrowMesh = Settings->GetArrowMesh())
		{
			if (GetStaticMesh() != ArrowMesh)
			{
				SetStaticMesh(ArrowMesh);
			}
            
			SetArrowColor(ArrowColor);
		}
	}
	
	SetArrowLength(ArrowLength);
}

void UFactoryArrowMeshComponent::SetArrowLength(float InLength)
{
	ArrowLength = InLength;
	SetRelativeScale3D(FVector(ArrowLength / 100.0f, ArrowSize, ArrowSize));
}

void UFactoryArrowMeshComponent::SetArrowColor(FColor NewColor)
{
	ArrowColor = NewColor;
	if (GetStaticMesh() && GetMaterial(0))
	{
		UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(GetMaterial(0));
		if (!DynMat)
		{
			DynMat = CreateAndSetMaterialInstanceDynamic(0);
		}
       
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(TEXT("ArrowColor"), FLinearColor(ArrowColor));
		}
	}
}

void UFactoryArrowMeshComponent::OnRegister()
{
	Super::OnRegister();
	UpdateVisuals();
}

void UFactoryArrowMeshComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateVisuals();
}
