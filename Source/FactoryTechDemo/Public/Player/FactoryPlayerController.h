#pragma once

#include "CoreMinimal.h"
#include "Player/Component/FactoryPlacementComponent.h"
#include "GameFramework/PlayerController.h"
#include "FactoryPlayerController.generated.h"

class UFactoryInputConfig;
class UFactoryQuickSlotComponent;
class UFactoryInteractionComponent;
class IFactoryInteractable;
class UFactoryInventoryComponent;
class UFactoryPlacementComponent;
class UInputMappingContext;
class AFactoryCharacter;
class AFactoryTopViewPawn;
class UFactoryObjectData;
class AFactoryPlacePreview;
class UFactoryInventoryWidget;
class UFactoryInteractionWidget;

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
    
    EFactoryViewModeType GetCurrentViewMode() const { return CurrentViewMode; }
    EPlacementMode GetCurrentPlacementMode() const {return PlacementComponent ? PlacementComponent->GetCurrentPlaceMode() : EPlacementMode::None;}
    bool GetIsInventoryOpen() const { return bIsInventoryOpen; }
    UFactoryInventoryComponent* GetInventoryComponent() const {return InventoryComponent;};
    
    UFactoryInputConfig* GetInputConfig() const { return InputConfig; }

protected:
    // --- 엔진 오버라이드 (Protected) ---
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    
    // --- 내부 로직 제어 ---
    void UpdateInputState();
    void OnToggleViewMode();
    
    void UpdateInputMappingContext() const;
    UFUNCTION()
    void OnPlacementModeChangedCallback(EPlacementMode PlacementMode);
    UFUNCTION()
    void QuickSlotExecuteCallback(UFactoryObjectData* ObjectData);

    // --- 상태 정보 (State) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory|State")
    EFactoryViewModeType CurrentViewMode = EFactoryViewModeType::NormalView;

    bool bIsInventoryOpen = false;

    // --- UI 구성 ---
    UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
    TSubclassOf<UFactoryInventoryWidget> InventoryWidgetBP;
    UPROPERTY(VisibleAnywhere, Category = "Factory|UI")
    TObjectPtr<UFactoryInventoryWidget> InventoryWidget;

    UPROPERTY(EditDefaultsOnly, Category = "Factory|UI")
    int32 InventoryColumns = 5;

    // --- 입력 에셋 (Enhanced Input) ---
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> BaseNormalViewContext;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> BaseTopViewContext;
    
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> NormalViewIdleActionContext;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> TopViewIdleActionContext;
    
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> GlobalIdleContext;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<UInputMappingContext> PlacementModeContext;
    
    UPROPERTY(EditDefaultsOnly, Category = "Factory|Input")
    TObjectPtr<UFactoryInputConfig> InputConfig;
    
    // --- 시스템 데이터 및 캐싱 ---
        // 컴포넌트
    UPROPERTY(EditDefaultsOnly, Category = "Factory|Component")
    TObjectPtr<UFactoryInventoryComponent> InventoryComponent;
    UPROPERTY(VisibleAnywhere, Category = "Factory|Component")
    TObjectPtr<UFactoryPlacementComponent> PlacementComponent;
    UPROPERTY(VisibleAnywhere, Category = "Factory|Component")
    TObjectPtr<UFactoryInteractionComponent> InteractionComponent;
    UPROPERTY(VisibleAnywhere, Category = "Factory|Component")
    TObjectPtr<UFactoryQuickSlotComponent> QuickSlotComponent;

    UPROPERTY()
    TWeakObjectPtr<AFactoryCharacter> CachedNormalViewCharacter;
    UPROPERTY()
    TWeakObjectPtr<AFactoryTopViewPawn> CachedTopViewPawn;
};