#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FactoryPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AFactoryCharacter;
class AFactoryTopViewPawn;
class UFactoryObjectData;
class AFactoryPlacePreview;

UENUM(BlueprintType)
enum class EFactoryViewModeType : uint8
{
    NormalView UMETA(DisplayName = "Normal Mode"),
    TopView UMETA(DisplayName = "Top View Mode"),
};

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
    EFactoryViewModeType CurrentViewMode = EFactoryViewModeType::NormalView;

private:
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;   // 이동관련
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> MouseMappingContext; // 마우스 회전 관련
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> QuickSlotContext; // 퀵슬롯 관련
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> PlacementContext; // 배치모드 관련

    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputAction> ToggleViewModeAction;    // 3인칭 - 탑뷰 전환 키
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputAction> PreviewObjectRotateAction;   // 프리뷰 객체 회전 키
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputAction> PlaceObjectAction;   // 프리뷰 객체 배치 키
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputAction> PlaceObjectCancelAction;   // 프리뷰 객체 배치 취소 키
    
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TArray<TObjectPtr<UInputAction>> QuickSlotActionArr;  // 0~9 퀵슬롯
    
    
    //시점 조작
    UPROPERTY()
    TWeakObjectPtr<AFactoryCharacter> CachedNormalViewCharacter;
    UPROPERTY()
    TWeakObjectPtr<AFactoryTopViewPawn> CachedTopViewPawn;
    
    
    UPROPERTY(EditAnywhere, Category = "Factory|Data")
    TArray<TObjectPtr<UFactoryObjectData>> QuickSlotObjectDataArr;
    UPROPERTY()
    TWeakObjectPtr<AFactoryPlacePreview> CurrentPlacePreview;
    
    UFUNCTION()
    void SetCurrentPlacePreview(UFactoryObjectData* Data);
    UPROPERTY()
    TObjectPtr<UFactoryObjectData> CurrentPlacePreviewData;

    
    void OnToggleViewMode();
    
    FVector GetPlacementObjectLocation() const;
    FVector CalculateSnappedLocation(FVector InRawLocation, FIntPoint InGridSize) const;
    void RotatePlacementPreview();
    void PlaceObject();
    void CancelPlaceObject();
    void ExecuteQuickSlotAction(int32 SlotIndex);
    
    const float MaxBuildTraceDistance = 1500.f;
    float GridLength = 100.f;
    bool bIsPlaceMode = false;
};