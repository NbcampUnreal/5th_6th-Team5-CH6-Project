#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class USphereComponent;
class APrototypeCharacter;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	class USphereComponent* InteractionSphere;

public:
	UInteractionComponent();

	void Initialize(APrototypeCharacter* InCharacter);
	void TryInteract();

	void RefreshInteractionUI();

	void ShowInteractionHint(FString Message, float Duration);

	void HandleDoorInteraction(AActor* DoorActor);
	void HandleItemInteraction(AActor* ItemActor);
	void HandleLeverInteraction(AActor* LeverActor);

	void AttachInteractingItem();
	void ConsumeInteractingItem();

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FVector CurrentPickupLocation;

	UPROPERTY()
	AActor* CurrentInteractingItem;

	bool bIsItemConsumed = false;
	FTransform InitialItemTransform;
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
	AActor* PendingDoorActor;

	UPROPERTY()
	AActor* LastInteractedDoorActor = nullptr;

	float LastDoorInteractTime = 0.0f;

public:
	void TriggerInteraction(); 
	void EndInteraction();

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bIsInteractingDoor = false;
};
