#include "FactoryPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FactoryCharacter.h"
#include "FactoryTopViewPawn.h"

AFactoryPlayerController::AFactoryPlayerController()
{
    // bShowMouseCursor = true;
}

void AFactoryPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
        if (MouseMappingContext)
        {
            Subsystem->AddMappingContext(MouseMappingContext, 1);
        }
    }

    CachedNormalViewCharacter = Cast<AFactoryCharacter>(GetCharacter());

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    CachedTopViewPawn = GetWorld()->SpawnActor<AFactoryTopViewPawn>(
        AFactoryTopViewPawn::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams);
    CachedTopViewPawn->SetActorHiddenInGame(true);
}

void AFactoryPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        // 뷰 모드 전환
        EnhancedInputComponent->BindAction(ToggleViewModeAction, ETriggerEvent::Started, this, &AFactoryPlayerController::OnToggleViewMode);
    }
}

void AFactoryPlayerController::OnToggleViewMode()
{
    if (!CachedNormalViewCharacter || !CachedTopViewPawn)
    {
        return;
    }

    const float BlendTime = 0.3f;
    APawn* CurrentPawn = GetPawn();
    APawn* TargetPawn = nullptr;
    EViewModeType NewMode;

    // 뷰 모드 전환 준비
    if (CurrentViewMode == EViewModeType::NormalView)
    {
        if (CurrentPawn)
        {
            CachedTopViewPawn->SetActorLocation(CurrentPawn->GetActorLocation());
        }
        CachedTopViewPawn->SetActorHiddenInGame(false);
        TargetPawn = CachedTopViewPawn;
        NewMode = EViewModeType::TopView;
    }
    else
    {
        CachedTopViewPawn->SetCameraPerspective(true);
        TargetPawn = CachedNormalViewCharacter;
        NewMode = EViewModeType::NormalView;
    }

    // 뷰 모드 전환 시작
    DisableInput(this);
    SetViewTargetWithBlend(TargetPawn, BlendTime);

    FTimerHandle ViewChangeTimerHandle;
    GetWorldTimerManager().SetTimer(ViewChangeTimerHandle, [this, TargetPawn, NewMode]()
                                    {
        if(!IsValid(this))
        {
            return;
        }

        Possess(TargetPawn);
        
        if (NewMode == EViewModeType::NormalView)
        {
            bShowMouseCursor = false;
            SetInputMode(FInputModeGameOnly());
        }
        else
        {
            CachedTopViewPawn->SetActorHiddenInGame(false);
            CachedTopViewPawn->SetCameraPerspective(false);
            bShowMouseCursor = true;
            SetInputMode(FInputModeGameAndUI());
        }
        EnableInput(this);
        CurrentViewMode = NewMode; }, BlendTime, false);
}