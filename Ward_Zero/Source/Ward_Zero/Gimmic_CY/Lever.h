#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "Gimmic_CY/Base/ObjectBase.h"
#include "Lever.generated.h"

class UTimelineComponent;
class ADoorBase;

UCLASS()
class WARD_ZERO_API ALever : public AObjectBase
{
	GENERATED_BODY()
	
public:
	ALever();

	void BeginPlay() override;
	
private:
	void LeverOpenDoor();
	void LeverCloseDoor();
	void LeverLockInteraction();
	void LeverUnLockInteraction();
	
	FOnTimelineFloat UpdateFunctionFloat;
	
	UFUNCTION()
	void UpdateTimelineFunction(float Output);
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* LeverTimelineFloatCurve;
	
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* LeverTimelineComp;
	
	FRotator InitialRotation;
	
	


public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetRoll = 80;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartRoll = -80;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* LeverHandle;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Doors For Open"), Category="Interaction Actors")
	TArray<ADoorBase*> DoorsForOpen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (DisplayName = "Doors For Close"), Category="Interaction Actors")
	TArray<ADoorBase*> DoorsForClose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (DisplayName = "Actors For Lock Interaction"), Category="Interaction Actors")
	TArray<AActor*> InteractionActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (DisplayName = "Actors For UnLock Interaction"), Category="Interaction Actors")
	TArray<AActor*> UnInteractionActors;
	
	EInteractionType GetInteractionType_Implementation() const override;

public:

	void ActivateLever();
	// ===== IGimmickInterface =====
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
};
