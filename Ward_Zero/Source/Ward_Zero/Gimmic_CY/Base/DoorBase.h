#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmic_CY/Base/ObjectBase.h"
#include "Components/TimelineComponent.h"
#include "DoorBase.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UNavModifierComponent;

UCLASS()
class WARD_ZERO_API ADoorBase : public AObjectBase
{
	GENERATED_BODY()
	
public:
	ADoorBase();
	UFUNCTION(BlueprintCallable)
	virtual void OpenDoor();
	
	UFUNCTION(BlueprintCallable)
	virtual void CloseDoor();
	
	virtual bool SetBCanInteract(bool IsCanInteract) override;
	
	virtual void Activate() override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:

	UPROPERTY()
	bool bCollected = false;

	// Timeline
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DoorTimelineComp;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DoorTimelineFloatCurve;

	FOnTimelineFloat UpdateFunctionFloat;

	UPROPERTY(VisibleAnywhere)
	UNavModifierComponent* NavModifier;
	
	
	// Lamp Mesh
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Lamp;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ChangeColorLampRed();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ChangeColorLampGreen();

	UFUNCTION()
	virtual void UpdateTimelineComp(float Output);

	bool bIsOpen = false;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* PickUpPoint;

	FVector GetInteractionTargetLocation_Implementation() const;

public:
	// ===== IGimmickInterface =====
	virtual EInteractionType GetInteractionType_Implementation() const override;

};
