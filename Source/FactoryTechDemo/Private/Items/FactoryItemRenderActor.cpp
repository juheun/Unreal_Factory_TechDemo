// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/FactoryItemRenderActor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Core/FactoryDeveloperSettings.h"
#include "Logistics/Belts/FactoryBelt.h"
#include "Subsystems/FactoryCycleSubsystem.h"
#include "Subsystems/FactoryDataSubsystem.h"


AFactoryItemRenderActor::AFactoryItemRenderActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AFactoryItemRenderActor::InitializeRenderers()
{
	const UFactoryDeveloperSettings* DevSettings = UFactoryDeveloperSettings::Get();
	UFactoryDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UFactoryDataSubsystem>();
	if (!DataSubsystem || !DevSettings) return;
	
	UStaticMesh* BaseMesh = DevSettings->GetDefaultItemMesh();
	UMaterialInterface* BaseMaterial = DevSettings->GetDefaultItemMaterial();
	
	if (!BaseMesh || !BaseMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemRenderActor: 디벨로퍼 세팅에 DefaultItemMesh 또는 Material이 없습니다!"));
		return;
	}
	
	TArray<EFactoryItemCategory> RenderCategories = { 
		EFactoryItemCategory::Resource, 
	};
	
	for (EFactoryItemCategory Category : RenderCategories)
	{
		TArray<UFactoryItemData*> Items = DataSubsystem->GetItemDatasByCategory(Category);
		for (UFactoryItemData* ItemData : Items)
		{
			if (!ItemData || !ItemData->ItemIcon) continue;

			UInstancedStaticMeshComponent* NewISM = NewObject<UInstancedStaticMeshComponent>(this);
			NewISM->RegisterComponent();
			NewISM->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

			NewISM->SetStaticMesh(BaseMesh);
			NewISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			NewISM->SetCastShadow(true);
			NewISM->SetReceivesDecals(false);

			// 다이내믹 머티리얼 생성 및 아이콘 텍스처 삽입
			UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			MID->SetTextureParameterValue(FName("ItemIcon"), ItemData->ItemIcon);
			NewISM->SetMaterial(0, MID);

			ItemISMMap.Add(ItemData, NewISM);
			RenderRequests.Add(ItemData, TArray<FTransform>());
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("ItemRenderActor 초기화 완료: 총 %d개의 ISM이 생성되었습니다."), ItemISMMap.Num());
}

void AFactoryItemRenderActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 사이클 알파 추출
	float Alpha = 0.f;
	if (UFactoryCycleSubsystem* CycleSubsystem = GetWorld()->GetSubsystem<UFactoryCycleSubsystem>())
	{
		Alpha = CycleSubsystem->GetCycleAlpha();
	}
	// 활성화된 벨트들에게서만 랜더링 요청 긁어오기
	for (int32 i = ActiveBelts.Num() - 1; i >= 0; --i)
	{
		if (AFactoryBelt* Belt = ActiveBelts[i].Get())
		{
			const UFactoryItemData* ItemData = Belt->GetCurrentItemData();
			if (ItemData)
			{
				FTransform Transform = Belt->GetItemRenderTransform(Alpha);
				RequestRenderItem(ItemData, Transform);
			}
		}
		else
		{
			// 혹시라도 벨트가 파괴되었는데 해제가 누락된 경우 청소
			ActiveBelts.RemoveAtSwap(i); 
		}
	}
	// 랜더린 요청 처리
	for (auto& Pair : RenderRequests)
	{
		const UFactoryItemData* ItemData = Pair.Key;
		TArray<FTransform>& Transforms = Pair.Value;

		if (UInstancedStaticMeshComponent** FoundISM = ItemISMMap.Find(ItemData))
		{
			UInstancedStaticMeshComponent* ISM = *FoundISM;
			
			int32 RequiredCount = Transforms.Num();
			int32 CurrentCount = ISM->GetInstanceCount();
			
			// 필요한 갯수와 현재 인스턴스 갯수 맞추기
			if (CurrentCount < RequiredCount)
			{
				// 모자라면 부족한 만큼 기본 위치(0,0,0)에 추가
				for (int32 i = CurrentCount; i < RequiredCount; ++i)
				{
					ISM->AddInstance(FTransform::Identity);
				}
			}
			else if (CurrentCount > RequiredCount)
			{
				// 남으면 끝에서부터 삭제
				for (int32 i = CurrentCount - 1; i >= RequiredCount; --i)
				{
					ISM->RemoveInstance(i);
				}
			}

			if (RequiredCount > 0)
			{
				ISM->BatchUpdateInstancesTransforms(0, Transforms, true, true, false); 
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ItemRenderActor: %s ItemData 정보가 없습니다"), *ItemData->ItemName.ToString());
		}

		// 다음 프레임을 위해 리셋 (메모리 재할당 방지)
		Transforms.Reset();
	}
}

void AFactoryItemRenderActor::RequestRenderItem(const UFactoryItemData* ItemData, const FTransform& Transform)
{
	if (ItemData && RenderRequests.Contains(ItemData))
	{
		RenderRequests[ItemData].Add(Transform);
	}
}

void AFactoryItemRenderActor::RegisterActiveBelt(AFactoryBelt* Belt)
{
	if (Belt)
	{
		ActiveBelts.AddUnique(Belt);
	}
}

void AFactoryItemRenderActor::UnRegisterActiveBelt(AFactoryBelt* Belt)
{
	if (Belt)
	{
		ActiveBelts.RemoveSwap(Belt); 
	}
}

