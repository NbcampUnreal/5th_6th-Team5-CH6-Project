#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionBase.generated.h"

class UWardSaveGame;

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	None,
	Door,
	Ammo,
	Document,  
	Save,
	Key,
	Heal,
	Lever
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

	virtual bool SetBCanInteract(bool IsCanInteract) = 0;

	virtual bool GetBCanInteract() const  = 0;
	
	virtual void SaveActorState() const = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowPressEWidget();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HidePressEWidget();

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	FVector GetInteractionTargetLocation() const;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction|IK")
	FVector GetIKTargetLocation() const;

protected:

	bool bCanInteract = true;
};
