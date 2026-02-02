#pragma once

#include "CoreMinimal.h"
#include "ViewModeType.generated.h"

UENUM(BlueprintType)
enum class EViewModeType : uint8
{
    NormalView UMETA(DisplayName = "Normal Mode"),
    TopView UMETA(DisplayName = "Top View Mode"),
};