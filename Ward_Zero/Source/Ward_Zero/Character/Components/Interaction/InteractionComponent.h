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
	void HandleLeverInteraction(AActor* LeverActor);

	void AttachInteractingItem();
	void ConsumeInteractingItem();

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FVector CurrentPickupLocation;

	UPROPERTY()
	AActor* CurrentInteractingItem;
private:
	UFUNCTION()
	void OnInteractableBeganOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnInteractableEndedOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex
	);

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

public:
	void TriggerInteraction(); 
	void EndInteraction();
};
