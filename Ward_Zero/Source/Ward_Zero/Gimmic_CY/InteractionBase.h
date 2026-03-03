#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionBase.generated.h"

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	None,
	Door,
	Ammo
};

UINTERFACE(MinimalAPI)
class UInteractionBase : public UInterface
{
	GENERATED_BODY()
};

class APrototypeCharacter;

class WARD_ZERO_API IInteractionBase
{
	GENERATED_BODY()

public:

	virtual void OnIneractionRangeEntered() = 0;
	virtual void OnIneractionRangeExited() = 0;
	virtual void OnIneracted(APrototypeCharacter* Character) = 0;
	virtual void HandleInteraction(APrototypeCharacter* Character) = 0;
	virtual bool CanBeInteracted() const = 0;
};
