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
    
    // 4. 델리게이트 등록
    if (PlacementComponent)
    {
        PlacementComponent->OnPlacementModeChanged.AddDynamic(this, &AFactoryPlayerController::SetPlacementMappingContext);
    }
    if (QuickSlotComponent)
    {
        QuickSlotComponent->OnQuickSlotExecuted.AddDynamic(this, &AFactoryPlayerController::ExecuteQuickSlotAction);
    }
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
    }, BlendTime, false);
}

void AFactoryPlayerController::SetPlacementMappingContext(EPlacementMode PlacementMode)
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (PlacementMode == EPlacementMode::None)
        {
            Subsystem->RemoveMappingContext(PlacementContext);
        }
        else
        {
            Subsystem->AddMappingContext(PlacementContext, 2);
        }
    }
}

void AFactoryPlayerController::ExecuteQuickSlotAction(UFactoryObjectData* ObjectData)
{
    if (!PlacementComponent || PlacementComponent->GetCurrentPlaceMode() != EPlacementMode::None) return;
    if (ObjectData)
    {
        PlacementComponent->SetPlaceFromDataPreview(ObjectData);
    }
}

