#include "Player/FactoryPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UI/Inventory/FactoryInventoryWidget.h"
#include "Player/Component/FactoryInventoryComponent.h"
#include "Player/FactoryCharacter.h"
#include "Player/FactoryTopViewPawn.h"
#include "Player/Component/FactoryInteractionComponent.h"
#include "Player/Component/FactoryPlacementComponent.h"
#include "Player/Component/FactoryQuickSlotComponent.h"
#include "Player/Input/FactoryInputConfig.h"

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

    // 액터 캐싱 및 생성
    CachedNormalViewCharacter = Cast<AFactoryCharacter>(GetCharacter());
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    CachedTopViewPawn = GetWorld()->SpawnActor<AFactoryTopViewPawn>(
        AFactoryTopViewPawn::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    CachedTopViewPawn->SetActorHiddenInGame(true);
    
    // UI 초기화
    if (IsLocalController() && InventoryWidgetBP)
    {
        InventoryWidget = CreateWidget<UFactoryInventoryWidget>(this, InventoryWidgetBP);
        if (InventoryWidget && InventoryComponent)
        {
            InventoryWidget->InitInventory(InventoryComponent, InventoryColumns);
        }
    }
    
    // 델리게이트 등록
    if (PlacementComponent)
    {
        PlacementComponent->OnPlacementModeChanged.AddDynamic(this, &AFactoryPlayerController::OnPlacementModeChangedCallback);
    }
    if (QuickSlotComponent)
    {
        QuickSlotComponent->OnQuickSlotExecuted.AddDynamic(this, &AFactoryPlayerController::QuickSlotExecuteCallback);
    }
    
    UpdateInputMappingContext();
}

void AFactoryPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    
    EPlacementMode PlacementMode = EPlacementMode::None;
    
    if (PlacementComponent)
    {
        PlacementComponent->UpdatePreviewState();
        PlacementMode = PlacementComponent->GetCurrentPlaceMode();
    }
    
    if (InteractionComponent)
    {
        InteractionComponent->UpdateInteraction();
    }
}

void AFactoryPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        if (!InputConfig) return;
        
        EnhancedInputComponent->BindAction(InputConfig->ToggleViewModeAction, ETriggerEvent::Started, this, &AFactoryPlayerController::OnToggleViewMode);
        EnhancedInputComponent->BindAction(InputConfig->ToggleInventoryAction, ETriggerEvent::Started, this, &AFactoryPlayerController::ToggleInventoryWidget);
        
        if (PlacementComponent)
        {
            PlacementComponent->SetUpInputComponent(EnhancedInputComponent, InputConfig);
        }
        
        if (InteractionComponent)
        {
            InteractionComponent->SetUpInputComponent(EnhancedInputComponent, InputConfig);
        }
        
        if (QuickSlotComponent)
        {
            QuickSlotComponent->SetUpInputComponent(EnhancedInputComponent, InputConfig);
        }
    }
}

#pragma endregion 

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
        UpdateInputMappingContext();
    }, BlendTime, false);
}

void AFactoryPlayerController::UpdateInputMappingContext() const
{
    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UEnhancedInputLocalPlayerSubsystem is Null"));
        return;
    }
    
    Subsystem->RemoveMappingContext(BaseNormalViewContext);
    Subsystem->RemoveMappingContext(BaseTopViewContext);
    
    Subsystem->RemoveMappingContext(NormalViewIdleActionContext);
    Subsystem->RemoveMappingContext(TopViewIdleActionContext);
    
    Subsystem->RemoveMappingContext(GlobalIdleContext);
    Subsystem->RemoveMappingContext(PlacementModeContext);
    
    if (CurrentViewMode == EFactoryViewModeType::NormalView)
    {
        Subsystem->AddMappingContext(BaseNormalViewContext, 0);
    }
    else
    {
        Subsystem->AddMappingContext(BaseTopViewContext, 0);
    }
    
    if (GetCurrentPlacementMode() == EPlacementMode::None)
    {
        if (CurrentViewMode == EFactoryViewModeType::NormalView)
        {
            Subsystem->AddMappingContext(NormalViewIdleActionContext, 1);
        }
        else
        {
            Subsystem->AddMappingContext(TopViewIdleActionContext, 1);
        }
        Subsystem->AddMappingContext(GlobalIdleContext, 2);
    }
    else
    {
        Subsystem->AddMappingContext(PlacementModeContext, 2);
    }
}

void AFactoryPlayerController::OnPlacementModeChangedCallback(EPlacementMode PlacementMode)
{
    UpdateInputMappingContext();
}

void AFactoryPlayerController::QuickSlotExecuteCallback(UFactoryObjectData* ObjectData)
{
    if (!PlacementComponent || PlacementComponent->GetCurrentPlaceMode() != EPlacementMode::None) return;
    if (ObjectData)
    {
        PlacementComponent->SetPlaceFromDataPreview(ObjectData);
    }
}

