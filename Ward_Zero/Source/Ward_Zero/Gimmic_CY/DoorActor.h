#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Gimmic_CY/InteractionBase.h"
#include "DoorActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class WARD_ZERO_API ADoorActor : public AActor, public IInteractionBase
{
	GENERATED_BODY()
	
public:	
	ADoorActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

protected:
	// Mesh
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DoorFrame;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Door;

	// Interaction Range
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* InteractionBox;

	// Timeline
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DoorTimelineComp;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DoorTimelineFloatCurve;

	FOnTimelineFloat UpdateFunctionFloat;

	bool bIsOpen = false;

	UFUNCTION()
	void UpdateTimelineComp(float Output);

	// 목표 회전값
	float TargetYaw = 90.f;

	// 문 기본 회전값 저장
	FRotator InitialRotation;

	// ===== IGimmickInterface =====
public:
	virtual void OnIneractionRangeEntered() override;
	virtual void OnIneractionRangeExited() override;
	virtual void OnIneracted(APrototypeCharacter* Character) override;
	virtual void HandleInteraction(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted() const override { return true; }

private:
	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);


};
