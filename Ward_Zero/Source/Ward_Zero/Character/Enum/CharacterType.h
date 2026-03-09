#pragma once

#include "CoreMinimal.h"
#include "CharacterType.generated.h"

UENUM(BlueprintType)
enum class EPlayerHitDirection : uint8
{
	Front,
	Back,
	Left,
	Right
};