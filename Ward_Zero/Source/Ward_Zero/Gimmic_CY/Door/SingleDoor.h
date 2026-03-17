#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Base/DoorBase.h"
#include "SingleDoor.generated.h"

class UStaticMeshComponent;
class UNavModifierComponent;
class USceneComponent;

UCLASS()
class WARD_ZERO_API ASingleDoor : public ADoorBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	ASingleDoor();

	// ===== IGimmickInterface =====
	virtual bool CanBeInteracted_Implementation() const override { return true; }
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;

protected:

	UPROPERTY(VisibleAnywhere)
	USceneComponent* PickUpPoint;

	float TargetYaw = 90.f;

	bool bIsOpen = false;

	FVector GetInteractionTargetLocation_Implementation() const;

	FRotator InitialRotation;

	UFUNCTION()
	void UpdateTimelineComp(float Output);
};
