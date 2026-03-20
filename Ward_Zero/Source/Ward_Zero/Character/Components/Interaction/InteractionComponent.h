#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UBoxComponent;
class APrototypeCharacter;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	void Initialize(APrototypeCharacter* InCharacter, UBoxComponent* InBox);
	void TryInteract();

	void HandleDoorInteraction(AActor* DoorActor);
	void HandleItemInteraction(AActor* ItemActor);

	void AttachInteractingItem();
	void ConsumeInteractingItem();

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FVector CurrentPickupLocation;

private:

	UPROPERTY()
	APrototypeCharacter* OwnerCharacter;

	UPROPERTY()
	UBoxComponent* InteractableBox;

	UPROPERTY()
	AActor* PendingDoorActor;

	UPROPERTY()
	AActor* LastInteractedDoorActor = nullptr;

	float LastDoorInteractTime = 0.0f;
	bool bIsInteractingDoor = false;

	UPROPERTY()
	AActor* CurrentInteractingItem;

};
