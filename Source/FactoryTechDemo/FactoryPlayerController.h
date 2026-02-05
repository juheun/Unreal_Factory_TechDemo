#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ViewModeType.h"
#include "FactoryPlayerController.generated.h"

/**
 * 팩토리 건설 및 시점 제어를 담당하는 컨트롤러
 */
UCLASS()
class FACTORYTECHDEMO_API AFactoryPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AFactoryPlayerController();
    
    virtual void PlayerTick(float DeltaTime) override;

    //void SetCurrentPlacementObject(class UFactoryObjectData* NewData);

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory|State")
    EViewModeType CurrentViewMode = EViewModeType::NormalView;

private:
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<class UInputMappingContext> DefaultMappingContext;   // 이동관련
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<class UInputMappingContext> MouseMappingContext; // 마우스 회전 관련

    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<class UInputAction> ToggleViewModeAction;    // 3인칭 - 탑뷰 전환 버튼
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<class UInputAction> PreviewObjectRotateAction;   // 프리뷰 객체 회전 버튼
    
    // 테스트 후 삭제
    UPROPERTY(EditAnywhere)
    TObjectPtr<class UFactoryObjectData> TempObjectData;

    UPROPERTY()
    TObjectPtr<class AFactoryCharacter> CachedNormalViewCharacter;
    UPROPERTY()
    TObjectPtr<class AFactoryTopViewPawn> CachedTopViewPawn;
    
    UPROPERTY()
    TObjectPtr<class AFactoryPlacePreview> CurrentPlacePreview;
    
    UFUNCTION()
    void SetCurrentPlacementPreview(class UFactoryObjectData* data);
    UPROPERTY()
    TObjectPtr<class UFactoryObjectData> CurrentPlacePreviewData;

    const float MaxBuildTraceDistance = 1500.f;
    
    void OnToggleViewMode();
    void RotatePlacementPreview();
    
    FVector GetPlacementObjectLocation() const;
    
    FVector CalculateSnappedLocation(FVector InRawLocation, FIntPoint InGridSize) const;
    
    float GridLength = 100.f;
};