// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/Components/FactoryRecipeBillboardComponent.h"

#include "Items/FactoryItemData.h"
#include "Items/FactoryRecipeData.h"
#include "Kismet/GameplayStatics.h"
#include "UI/World/Widgets/FactoryRecipeBillboardWidget.h"


UFactoryRecipeBillboardComponent::UFactoryRecipeBillboardComponent()
{
	SetDrawSize(FVector2D(100.f, 100.f));
}

void UFactoryRecipeBillboardComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CachedZOffset = DefaultBillboardZHeight;

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
			CachedZOffset = (MeshBounds.Max.Z - OwnerActor->GetActorLocation().Z) + BillboardZHeight;
		}
	}
}

void UFactoryRecipeBillboardComponent::WakeUp()
{
	Super::WakeUp();
	RefreshIconUI();
}

void UFactoryRecipeBillboardComponent::OnRecipeChangedCallback(const UFactoryRecipeData* NewRecipe)
{
	UTexture2D* Icon = (NewRecipe && NewRecipe->Output.ItemData) ? NewRecipe->Output.ItemData->ItemIcon : nullptr;
	SetIcon(Icon);
}

void UFactoryRecipeBillboardComponent::OnItemChangedCallback(const UFactoryItemData* NewItemData)
{
	UTexture2D* Icon = (NewItemData && NewItemData->ItemIcon) ? NewItemData->ItemIcon : nullptr;
	SetIcon(Icon);
}

void UFactoryRecipeBillboardComponent::SetIcon(UTexture2D* Icon)
{
	if (!GetUserWidgetObject())
	{
		InitWidget();
	}
	
	UFactoryRecipeBillboardWidget* BillboardWidget = Cast<UFactoryRecipeBillboardWidget>(GetUserWidgetObject());
	if (!BillboardWidget) return;
	
	if (Icon)
	{
		BillboardWidget->SetRecipeIcon(Icon);
		bIsIconSet = true;
	}
	else
	{
		bIsIconSet = false;
	}
	
	if (bIsAwake)
	{
		RefreshIconUI();
	}
}

void UFactoryRecipeBillboardComponent::RefreshIconUI()
{
	UFactoryRecipeBillboardWidget* BillboardWidget = Cast<UFactoryRecipeBillboardWidget>(GetUserWidgetObject());
	if (!BillboardWidget) return;
	
	if (bIsIconSet)
	{
		BillboardWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		BillboardWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFactoryRecipeBillboardComponent::UpdateUIPlacement(float DeltaTime, const FVector& CameraLoc, const FVector& CameraForward, const FVector& OwnerLoc)
{
	FVector TargetLoc = OwnerLoc;
	
	TargetLoc.Z += CachedZOffset;
	
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
