#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmic_CY/Object/ObjectBase.h"
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
	
	
	virtual void Activate() override;
	
	bool GetbIsOpen() const{return bIsOpen;}


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
	
	
	
	

	UFUNCTION()
	virtual void UpdateTimelineComp(float Output);

	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void PostInitializeComponents() override;
	bool bIsOpen = false;

	

	virtual FVector GetInteractionTargetLocation_Implementation() const;
	
	

public:
	// ===== IGimmickInterface =====
	virtual EInteractionType GetInteractionType_Implementation() const override;

};
