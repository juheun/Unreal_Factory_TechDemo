#include "Player/FactoryPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Items/FactoryFacilityItemData.h"
#include "Player/Component/FactoryInventoryComponent.h"
#include "Player/FactoryCharacter.h"
#include "Player/FactoryTopViewPawn.h"
#include "Player/Component/FactoryInteractionComponent.h"
#include "Player/Component/FactoryPlacementComponent.h"
#include "Player/Component/FactoryPlayerContextHUDComponent.h"
#include "Player/Component/FactoryQuickSlotComponent.h"
#include "Player/Component/FactoryWorldUIActivatorComponent.h"
#include "Player/Input/FactoryInputConfig.h"
#include "UI/Storage/FactoryStorageMenuWidget.h"

AFactoryPlayerController::AFactoryPlayerController()
{
    InventoryComponent = CreateDefaultSubobject<UFactoryInventoryComponent>(TEXT("InventoryComponent"));
    PlacementComponent = CreateDefaultSubobject<UFactoryPlacementComponent>(TEXT("PlacementComponent"));
    InteractionComponent = CreateDefaultSubobject<UFactoryInteractionComponent>(TEXT("InteractionComponent"));
    QuickSlotComponent = CreateDefaultSubobject<UFactoryQuickSlotComponent>(TEXT("QuickSlotComponent"));
    ContextHUDComponent = CreateDefaultSubobject<UFactoryPlayerContextHUDComponent>(TEXT("ContextHUDComponent"));
    WorldUIActivatorComponent = CreateDefaultSubobject<UFactoryWorldUIActivatorComponent>(TEXT("WorldUIActivatorComponent"));
}

#pragma region 엔진 라이프 사이클

void AFactoryPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 액터 캐싱 및 생성
    CachedNormalViewCharacter = Cast<AFactoryCharacter>(GetCharacter());
    
    // WorldUIActivatorComponent를 노말 뷰 캐릭터에 부착
    if (AFactoryCharacter* NormalViewCharacter = CachedNormalViewCharacter.Get())
    {
        if (WorldUIActivatorComponent)
        {
            WorldUIActivatorComponent->AttachToComponent(NormalViewCharacter->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        }
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    CachedTopViewPawn = GetWorld()->SpawnActor<AFactoryTopViewPawn>(
        AFactoryTopViewPawn::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    CachedTopViewPawn->SetActorHiddenInGame(true);
    
    if (IsLocalController() && StorageMenuWidgetBP)
    {
        StorageMenuWidget = CreateWidget<UFactoryStorageMenuWidget>(this, StorageMenuWidgetBP);
        if (StorageMenuWidget)
        {
            StorageMenuWidget->AddToViewport();
            StorageMenuWidget->SetVisibility(ESlateVisibility::Collapsed); // 처음엔 숨기기
        }
    }
    
    // 델리게이트 등록
    if (PlacementComponent)
    {
        PlacementComponent->OnPlacementModeChanged.AddDynamic(this, &AFactoryPlayerController::OnPlacementModeChangedCallback);
        PlacementComponent->OnObjectPlacedFromInventorySignature.AddDynamic(this, &AFactoryPlayerController::HandleObjectPlacedFromInventory);
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
    
    if (PlacementComponent)
    {
        PlacementComponent->UpdatePreviewState();
    }
    if (InteractionComponent)
    {
        InteractionComponent->UpdateInteractionTextList();
    }
}

void AFactoryPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        if (!InputConfig) return;
        
        EnhancedInputComponent->BindAction(InputConfig->ToggleViewModeAction, ETriggerEvent::Started, this, &AFactoryPlayerController::OnToggleViewMode);
        EnhancedInputComponent->BindAction(InputConfig->ToggleInventoryAction, ETriggerEvent::Started, this, &AFactoryPlayerController::ToggleStorageMenu);
        
        if (PlacementComponent) PlacementComponent->SetUpInputComponent(EnhancedInputComponent, InputConfig);
        if (InteractionComponent) InteractionComponent->SetUpInputComponent(EnhancedInputComponent, InputConfig);
        if (QuickSlotComponent) QuickSlotComponent->SetUpInputComponent(EnhancedInputComponent, InputConfig);
    }
}

void AFactoryPlayerController::OnPossess(APawn* PossessedPawn)
{
    Super::OnPossess(PossessedPawn);
    
    if (PossessedPawn && WorldUIActivatorComponent)
    {
        WorldUIActivatorComponent->AttachToComponent(PossessedPawn->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }
}


#pragma endregion 

void AFactoryPlayerController::ToggleStorageMenu()
{
    bIsStorageMenuOpen = !bIsStorageMenuOpen;
    
    if (StorageMenuWidget)
    {
        if (bIsStorageMenuOpen)
        {
            StorageMenuWidget->OpenMenu(InventoryComponent, EFactoryMenuMode::Warehouse); 
            StorageMenuWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
        else
        {
            StorageMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
    
    UpdateInputState();
}


void AFactoryPlayerController::OpenFacilityMenu(AFactoryPlaceObjectBase* Facility, EFactoryMenuMode Mode)
{
    if (!Facility || !StorageMenuWidget) return;

    // 설비 메뉴가 이미 열려있는 상태인지 여부 상관없이, 무조건 해당 설비로 켬
    bIsStorageMenuOpen = true; 
    
    StorageMenuWidget->OpenMenu(InventoryComponent, Mode, Facility);
    StorageMenuWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    
    UpdateInputState();
}

void AFactoryPlayerController::HandleObjectPlacedFromInventory(const UFactoryFacilityItemData* FacilityItemData,
    int32 Amount)
{
    if (InventoryComponent && FacilityItemData)
    {
        InventoryComponent->AutoRemoveItem(FacilityItemData, Amount);
    }
}

void AFactoryPlayerController::UpdateInputState()
{
    if (bIsStorageMenuOpen)
    {
        FInputModeGameAndUI InputMode;
        if (StorageMenuWidget) InputMode.SetWidgetToFocus(StorageMenuWidget->TakeWidget());
        
        InputMode.SetHideCursorDuringCapture(false);
        
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
    else
    {
        if (CurrentViewMode == EFactoryViewModeType::TopView)
        {
            FInputModeGameAndUI InputMode;
            InputMode.SetHideCursorDuringCapture(false);
            SetInputMode(InputMode);
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
        OnViewModeChanged.Broadcast(CurrentViewMode);
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
    Subsystem->RemoveMappingContext(MultipleControlModeContext);
    
    if (CurrentViewMode == EFactoryViewModeType::NormalView)
    {
        Subsystem->AddMappingContext(BaseNormalViewContext, 0);
    }
    else
    {
        Subsystem->AddMappingContext(BaseTopViewContext, 0);
    }
    
    EPlacementMode CurrentPlacementMode = GetCurrentPlacementMode();
    if (CurrentPlacementMode == EPlacementMode::MultipleControl && CurrentViewMode == EFactoryViewModeType::TopView)
    {
        Subsystem->AddMappingContext(PlacementModeContext, 1);
        Subsystem->AddMappingContext(MultipleControlModeContext, 2);
    }
    else if (CurrentPlacementMode == EPlacementMode::None)
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
    if (ContextHUDComponent)
        ContextHUDComponent->OnPlacementModeChanged(PlacementMode);
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

