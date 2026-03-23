#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Object/Door/DoorBase.h"
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
	virtual EInteractionType GetInteractionType_Implementation() const override;
	ESingleDoorAnimationType GetSingleDoorAnimationType() const;

protected:

	float TargetYaw = -90.f;

	FRotator InitialRotation;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (DisplayName = "Door Action Type"),Category = "Door Action")
	ESingleDoorAnimationType DoorAnimationType = ESingleDoorAnimationType::SingleDoor_Push;
	

	virtual void UpdateTimelineComp(float Output) override;


public:
	
	
	ESingleDoorAnimationType GetDoorAnimationType();
	virtual void OpenDoor() override;
	virtual void CloseDoor() override;
};
