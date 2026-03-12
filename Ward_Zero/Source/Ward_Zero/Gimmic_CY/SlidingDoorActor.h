#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Gimmic_CY/InteractionBase.h"
#include "SlidingDoorActor.generated.h"

class UStaticMeshComponent;
class UTimelineComponent;
class UCurveFloat;
class APrototypeCharacter;

UCLASS()
class WARD_ZERO_API ASlidingDoorActor : public AActor, public IInteractionBase
{
	GENERATED_BODY()
	
public:	
	ASlidingDoorActor();

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DoorFrame;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Door;

	UPROPERTY()
	UTimelineComponent* DoorTimeline;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DoorCurve;

	FOnTimelineFloat UpdateFunctionFloat;

	UFUNCTION()
	void UpdateTimeline(float Value);

	// À§Ä¡
	FVector ClosedLocation;

	UPROPERTY(EditAnywhere)
	FVector OpenOffset = FVector(120.f, 0.f, 0.f);

	bool bIsOpen = false;

public:

	// Interaction Interface
	virtual void OnIneracted_Implementation(APrototypeCharacter* Character) override;
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted_Implementation() const override;
	virtual EInteractionType GetInteractionType_Implementation() const override;
	virtual bool SetBCanInteract(bool IsCanInteract) override;
	virtual bool GetBCanInteract() const override;
};
