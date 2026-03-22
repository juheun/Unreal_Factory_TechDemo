// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/FactoryRecipeBillboardComponent.h"

#include "Items/FactoryItemData.h"
#include "Items/FactoryRecipeData.h"
#include "UI/World/FactoryRecipeBillboardWidget.h"


UFactoryRecipeBillboardComponent::UFactoryRecipeBillboardComponent()
{
	SetDrawSize(FVector2D(100.f, 100.f));
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
		BillboardWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		BillboardWidget->SetRecipeIcon(Icon);
	}
	else
	{
		BillboardWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFactoryRecipeBillboardComponent::UpdateUIPlacement(float DeltaTime, const FVector& CameraLoc, const FVector& CameraForward, const FVector& OwnerLoc)
{
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
			TargetLoc.Z = MeshBounds.Max.Z + 150.f;
		}
		else
		{
			TargetLoc.Z += BillboardZHeight;
		}
	}
	else
	{
		TargetLoc.Z += BillboardZHeight; // 예외 처리 (기본값)
	}
    
	// 카메라 보게하기
	FRotator CameraRot = CameraForward.Rotation();
	FRotator TargetRot = FRotator(0.f, CameraRot.Yaw + 180.f, 0.f);

	SetWorldLocationAndRotation(TargetLoc, TargetRot);
}
