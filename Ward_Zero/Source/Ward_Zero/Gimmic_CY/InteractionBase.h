#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionBase.generated.h"

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	None,
	Door,
	Ammo,
	Document,  
	Save      
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

	UFUNCTION(BlueprintNativeEvent)
	void OnIneractionRangeEntered();

	UFUNCTION(BlueprintNativeEvent)
	void OnIneractionRangeExited();

	UFUNCTION(BlueprintNativeEvent)
	void OnIneracted(APrototypeCharacter* Character);

	UFUNCTION(BlueprintNativeEvent)
	void HandleInteraction(APrototypeCharacter* Character);

	UFUNCTION(BlueprintNativeEvent)
	bool CanBeInteracted() const;

	UFUNCTION(BlueprintNativeEvent)
	EInteractionType GetInteractionType() const;
};
