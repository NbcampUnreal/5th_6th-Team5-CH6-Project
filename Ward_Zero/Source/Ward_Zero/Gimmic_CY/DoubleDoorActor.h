#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Gimmic_CY/InteractionBase.h"
#include "DoubleDoorActor.generated.h"

class UBoxComponent;

UCLASS()
class WARD_ZERO_API ADoubleDoorActor : public AActor, public IInteractionBase
{
	GENERATED_BODY()
	
public:	
	ADoubleDoorActor();

protected:
	virtual void BeginPlay() override;

protected:

	// Root
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	// Doors
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* LeftDoor;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RightDoor;

	// Timeline
	UPROPERTY()
	UTimelineComponent* DoorTimeline;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DoorCurve;

	FOnTimelineFloat TimelineUpdate;

	// Interaction Range
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* InteractionBox;

	bool bIsOpen = false;

	UFUNCTION()
	void UpdateDoor(float Value);

public:	

	virtual void Tick(float DeltaTime) override;
	
	// Interface
	virtual void OnIneractionRangeEntered_Implementation() override;
	virtual void OnIneractionRangeExited_Implementation() override;
	virtual void OnIneracted_Implementation(APrototypeCharacter* Character) override;
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted_Implementation() const override { return true; };
};
