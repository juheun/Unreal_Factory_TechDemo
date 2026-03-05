#include "Player/FactoryPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Inventory/FactoryInventoryWidget.h"
#include "Inventory/FactoryInventoryComponent.h"
#include "Player/FactoryCharacter.h"
#include "Player/FactoryTopViewPawn.h"
#include "Player/Component/FactoryInteractionComponent.h"
#include "Player/Component/FactoryPlacementComponent.h"
#include "Player/Component/FactoryQuickSlotComponent.h"

AFactoryPlayerController::AFactoryPlayerController()
{
    InventoryComponent = CreateDefaultSubobject<UFactoryInventoryComponent>(TEXT("InventoryComponent"));
    PlacementComponent = CreateDefaultSubobject<UFactoryPlacementComponent>(TEXT("PlacementComponent"));
    InteractionComponent = CreateDefaultSubobject<UFactoryInteractionComponent>(TEXT("InteractionComponent"));
    QuickSlotComponent = CreateDefaultSubobject<UFactoryQuickSlotComponent>(TEXT("QuickSlotComponent"));
}

#pragma region 엔진 라이프 사이클

void AFactoryPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 1. 인풋 서브시스템 설정
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext) Subsystem->AddMappingContext(DefaultMappingContext, 0);
        if (MouseMappingContext) Subsystem->AddMappingContext(MouseMappingContext, 1);
        if (QuickSlotContext) Subsystem->AddMappingContext(QuickSlotContext, 1);
    }
    
    // 2. 액터 캐싱 및 생성
    CachedNormalViewCharacter = Cast<AFactoryCharacter>(GetCharacter());
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    CachedTopViewPawn = GetWorld()->SpawnActor<AFactoryTopViewPawn>(
        AFactoryTopViewPawn::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    CachedTopViewPawn->SetActorHiddenInGame(true);
    
    // 3. UI 초기화
    if (IsLocalController() && InventoryWidgetBP)
    {
        InventoryWidget = CreateWidget<UFactoryInventoryWidget>(this, InventoryWidgetBP);
        if (InventoryWidget && InventoryComponent)
        {
            InventoryWidget->InitInventory(InventoryComponent, InventoryColumns);
        }
    }
}

void AFactoryPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    
    bool bIsPlaceMode = false;
    
    if (PlacementComponent)
    {
        PlacementComponent->UpdatePreviewState();
        bIsPlaceMode = PlacementComponent->GetCurrentPlaceMode() != EPlacementMode::None;
    }
    
    if (InteractionComponent)
    {
        InteractionComponent->UpdateInteraction(CurrentViewMode, bIsPlaceMode, bIsInventoryOpen);
    }
}

void AFactoryPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInputComponent->BindAction(ToggleViewModeAction, ETriggerEvent::Started, this, &AFactoryPlayerController::OnToggleViewMode);
        EnhancedInputComponent->BindAction(PreviewObjectRotateAction, ETriggerEvent::Started, this, &AFactoryPlayerController::RotatePlacementPreview);
        EnhancedInputComponent->BindAction(PlaceObjectAction, ETriggerEvent::Started, this, &AFactoryPlayerController::PlaceObject);
        EnhancedInputComponent->BindAction(PlaceObjectCancelAction, ETriggerEvent::Started, this, &AFactoryPlayerController::CancelPlaceObject);
        EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &AFactoryPlayerController::ToggleInventoryWidget);
        EnhancedInputComponent->BindAction(ToggleBeltPlaceModeAction, ETriggerEvent::Started, this, &AFactoryPlayerController::ToggleBeltPlaceMode);
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AFactoryPlayerController::OnInteract);

        for (int i = 0; i < QuickSlotActionArr.Num(); i++)
        {
            if (QuickSlotActionArr[i])
            {
                EnhancedInputComponent->BindAction(QuickSlotActionArr[i], ETriggerEvent::Started, this, &AFactoryPlayerController::ExecuteQuickSlotAction, i);
            }
        }
    }
}

#pragma endregion 

#pragma region UI 및 입력 상태 제어

void AFactoryPlayerController::ToggleInventoryWidget()
{
    if (InventoryWidget)
    {
        if (bIsInventoryOpen) InventoryWidget->RemoveFromParent();
        else InventoryWidget->AddToViewport();
    }
    bIsInventoryOpen = !bIsInventoryOpen;
    UpdateInputState();
}

void AFactoryPlayerController::UpdateInputState()
{
    if (bIsInventoryOpen)
    {
        FInputModeGameAndUI InputMode;
        if (InventoryWidget) InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
    else
    {
        if (CurrentViewMode == EFactoryViewModeType::TopView)
        {
            SetInputMode(FInputModeGameAndUI());
            bShowMouseCursor = true;
        }
        else
        {
            SetInputMode(FInputModeGameOnly());
            bShowMouseCursor = false;
        }
    }
}

#pragma endregion 

#pragma region 뷰 모드 전환

void AFactoryPlayerController::OnToggleViewMode()
{
    if (!CachedNormalViewCharacter.IsValid() || !CachedTopViewPawn.IsValid()) return;

    const float BlendTime = 0.3f;
    APawn* TargetPawn = (CurrentViewMode == EFactoryViewModeType::NormalView) ? 
        Cast<APawn>(CachedTopViewPawn.Get()) : Cast<APawn>(CachedNormalViewCharacter.Get());

    if (CurrentViewMode == EFactoryViewModeType::NormalView)
    {
        CachedTopViewPawn->SetActorLocation(GetPawn()->GetActorLocation());
        CachedTopViewPawn->SetActorHiddenInGame(false);
    }
    else
    {
        CachedTopViewPawn->SetCameraPerspective(true);
    }
    
    DisableInput(this);
    SetViewTargetWithBlend(TargetPawn, BlendTime);

    FTimerHandle ViewChangeTimerHandle;
    GetWorldTimerManager().SetTimer(ViewChangeTimerHandle, [this, TargetPawn]()
    {
        if(!IsValid(this)) return;

        Possess(TargetPawn);
        EFactoryViewModeType NewMode = (CurrentViewMode == EFactoryViewModeType::NormalView) ? 
            EFactoryViewModeType::TopView : EFactoryViewModeType::NormalView;

        if (NewMode == EFactoryViewModeType::NormalView)
        {
            CachedTopViewPawn->SetActorHiddenInGame(true);
        }
        else
        {
            CachedTopViewPawn->SetActorHiddenInGame(false);
            CachedTopViewPawn->SetCameraPerspective(false);
        }

        CurrentViewMode = NewMode; 
        UpdateInputState();
        EnableInput(this);
    }, BlendTime, false);
}

#pragma endregion 

#pragma region 배치 명령 래핑

void AFactoryPlayerController::ToggleBeltPlaceMode()
{
    if (PlacementComponent)
    {
        bool bIsBeltPlaceMode = PlacementComponent->ToggleBeltPlaceMode();
        SetPlacementMappingContext(bIsBeltPlaceMode);
    }
}

void AFactoryPlayerController::RotatePlacementPreview()
{
    if (PlacementComponent) PlacementComponent->RotatePlacementPreview();
}

void AFactoryPlayerController::PlaceObject()
{
    if (PlacementComponent) PlacementComponent->ProcessPlacementAction();
    if (PlacementComponent->GetCurrentPlaceMode() != EPlacementMode::BeltPlace)
    {
        SetPlacementMappingContext(false);
    }
}

void AFactoryPlayerController::CancelPlaceObject()
{
    if (PlacementComponent) PlacementComponent->CancelPlaceObject();
    SetPlacementMappingContext(false);
}

void AFactoryPlayerController::SetPlacementMappingContext(bool bEnable) const
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (bEnable)
        {
            Subsystem->AddMappingContext(PlacementContext, 2);
        }
        else
        {
            Subsystem->RemoveMappingContext(PlacementContext);
        }
    }
}

#pragma endregion 

#pragma region 퀵슬롯 명령 래핑

void AFactoryPlayerController::ExecuteQuickSlotAction(int32 SlotIndex)
{
    if (!PlacementComponent || PlacementComponent->GetCurrentPlaceMode() != EPlacementMode::None) return;
    if (QuickSlotComponent)
    {
        UFactoryObjectData* QuickSlotData = QuickSlotComponent->GetQuickSlotData(SlotIndex);
        if (QuickSlotData)
        {
            PlacementComponent->SetPlaceFromDataPreview(QuickSlotData);
            SetPlacementMappingContext(true);
        }
    }
}

#pragma endregion 

#pragma region 상호작용 명령 래핑

void AFactoryPlayerController::OnInteract()
{
    if (!PlacementComponent || PlacementComponent->GetCurrentPlaceMode() != EPlacementMode::None) return;
    if (bIsInventoryOpen) return;
    
    if (InteractionComponent)
    {
        InteractionComponent->PerformInteraction(GetPawn(), CurrentViewMode);
    }
}

#pragma endregion
