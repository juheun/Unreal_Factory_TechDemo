#pragma once

#include "CoreMinimal.h"
#include "FactoryViewModeType.generated.h"

UENUM(BlueprintType)
enum class EFactoryViewModeType : uint8
{
    NormalView UMETA(DisplayName = "Normal Mode"),
    TopView UMETA(DisplayName = "Top View Mode"),
};