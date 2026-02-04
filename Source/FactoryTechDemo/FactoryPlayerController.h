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
    TObjectPtr<class UInputMappingContext> DefaultMappingContext;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<class UInputMappingContext> MouseMappingContext;

    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<class UInputAction> ToggleViewModeAction;
    UPROPERTY(EditAnywhere, Category = "Factory|Input")
    TObjectPtr<class UInputAction> GhostRotateAction;

    UPROPERTY()
    TObjectPtr<class AFactoryCharacter> CachedNormalViewCharacter;
    UPROPERTY()
    TObjectPtr<class AFactoryTopViewPawn> CachedTopViewPawn;
    
    UPROPERTY()
    TObjectPtr<class AFactoryPlacePreview> CurrentPlacementPreview;

    const float MaxBuildTraceDistance = 1000.f;
    
    void OnToggleViewMode();
    void RotatePlacementPreview();
    
    FVector GetPlacementObjectLocation() const;
    
    FVector CalculateSnappedLocation(FVector InRawLocation, FIntPoint InGridSize) const;
};