#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FactoryPlayerController.generated.h"

class IFactoryInteractable;
class UFactoryInventoryComponent;
class UInputMappingContext;
class UInputAction;
class AFactoryCharacter;
class AFactoryTopViewPawn;
class UFactoryObjectData;
class AFactoryPlacePreview;
class UFactoryInventoryWidget;

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

    // --- 엔진 오버라이드 ---
    virtual void PlayerTick(float DeltaTime) override;

    // --- 외부 인터페이스 ---
    void ToggleInventoryWidget();   //인벤토리 UI 토글

protected:
    // --- 엔진 오버라이드 (Protected) ---
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    // --- 상태 정보 (State) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory|State")
    EFactoryViewModeType CurrentViewMode = EFactoryViewModeType::NormalView;

    bool bIsInventoryOpen = false;
    bool bIsPlaceMode = false;

    // --- UI 구성 ---
    UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
    TSubclassOf<UFactoryInventoryWidget> InventoryWidgetBP;

    UPROPERTY(VisibleAnywhere, Category = "Factory|UI")
    TObjectPtr<UFactoryInventoryWidget> InventoryWidget;

    UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
    int32 InventoryColumns = 5;

    // --- 입력 에셋 (Enhanced Input) ---
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> MouseMappingContext;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> QuickSlotContext;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> PlacementContext;

    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputAction> ToggleViewModeAction;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputAction> PreviewObjectRotateAction;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputAction> PlaceObjectAction;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputAction> PlaceObjectCancelAction;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputAction> ToggleInventoryAction;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputAction> InteractAction;

    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TArray<TObjectPtr<UInputAction>> QuickSlotActionArr;

    // --- 시스템 데이터 및 캐싱 ---
    UPROPERTY(EditDefaultsOnly, Category = "Factory|Component")
    TObjectPtr<UFactoryInventoryComponent> InventoryComponent;

    UPROPERTY(EditAnywhere, Category = "Factory|Data")
    TArray<TObjectPtr<UFactoryObjectData>> QuickSlotObjectDataArr;

    UPROPERTY()
    TWeakObjectPtr<AFactoryCharacter> CachedNormalViewCharacter;
    UPROPERTY()
    TWeakObjectPtr<AFactoryTopViewPawn> CachedTopViewPawn;

    UPROPERTY()
    TWeakObjectPtr<AFactoryPlacePreview> CurrentPlacePreview;
    UPROPERTY()
    TObjectPtr<UFactoryObjectData> CurrentPlacePreviewData;

    // --- 내부 로직 제어 ---
    void UpdateInputState();
    void OnToggleViewMode();

    // 배치 시스템
    UFUNCTION()
    void SetCurrentPlacePreview(UFactoryObjectData* Data);
    FVector GetPlacementObjectLocation() const;
    FVector CalculateSnappedLocation(FVector InRawLocation, FIntPoint InGridSize) const;
    void RotatePlacementPreview();
    void PlaceObject();
    void CancelPlaceObject();

    // 퀵슬롯 시스템
    void ExecuteQuickSlotAction(int32 SlotIndex);
    
    // 상호작용 시스템
    void OnInteract();
    TScriptInterface<IFactoryInteractable> FindBestInteractable();
    UPROPERTY(EditAnywhere, Category = "Factory|Interation")
    float InteractionRange = 300.f;

private:
    // --- 상수 및 설정값 ---
    const float MaxBuildTraceDistance = 1500.f;
    float GridLength = 100.f;
};