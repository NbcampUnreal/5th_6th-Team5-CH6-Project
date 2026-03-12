#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "Gimmic_CY/InteractionBase.h"
#include "DoorActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UNavModifierComponent;
class USceneComponent;

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
	// Root Scene
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* InteractionBox;

	//Scene
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Scene;

	// Door Mesh
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Door;

	// Timeline
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DoorTimelineComp;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DoorTimelineFloatCurve;

	FOnTimelineFloat UpdateFunctionFloat;

	bool bIsOpen = false;

	UFUNCTION()
	void UpdateTimelineComp(float Output);

	// ��ǥ ȸ����
	float TargetYaw = 90.f;

	// �� �⺻ ȸ���� ����
	FRotator InitialRotation;

	UPROPERTY(VisibleAnywhere)
	UNavModifierComponent* NavModifier;

	bool bCanInteract = true;

	// ===== IGimmickInterface =====
public:
	virtual void OnIneractionRangeEntered_Implementation() override;
	virtual void OnIneractionRangeExited_Implementation() override;
	virtual void OnIneracted_Implementation(APrototypeCharacter* Character) override;
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted_Implementation() const override { return true; }
	virtual EInteractionType GetInteractionType_Implementation() const override;
	virtual bool SetBCanInteract(bool IsCanInteract) override;
	virtual bool GetBCanInteract() const override;

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
