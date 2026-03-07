#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FactoryInteractable.generated.h"

enum class EPlacementMode : uint8;
// 엔진이 인터페이스를 인식하기 위한 메타데이터 클래스
UINTERFACE(MinimalAPI)
class UFactoryInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 실제 상호작용 로직을 구현할 인터페이스 클래스
 */
class FACTORYTECHDEMO_API IFactoryInteractable
{
	GENERATED_BODY()

public:
	virtual void Interact(const AActor* Interactor, const EPlacementMode CurrentMode) = 0;	// 플레이어(Interactor)가 상호작용했을 때 실행될 순수 가상 함수
	virtual bool TryGetInteractText(const EPlacementMode CurrentMode, FText& OutText) const = 0;	// 상호작용 UI에 표시할 텍스트 반환
};