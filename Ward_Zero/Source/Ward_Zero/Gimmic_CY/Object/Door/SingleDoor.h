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
	
	UFUNCTION(BlueprintCallable)
	ESingleDoorAnimationType GetSingleDoorAnimationType() const;

	
	private:
	UFUNCTION()
	void OnOverLapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverLapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	virtual FVector GetInteractionTargetLocation_Implementation() const override;
protected:

	float TargetYaw = -90.f;

	FRotator InitialRotation;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta = (DisplayName = "Door Action Type"),Category = "Setting")
	ESingleDoorAnimationType DoorAnimationType = ESingleDoorAnimationType::SingleDoor_Push;

	virtual void UpdateTimelineComp(float Output) override;


public:
	
	
	ESingleDoorAnimationType GetDoorAnimationType();
	virtual void OpenDoor() override;
	virtual void CloseDoor() override;
};
