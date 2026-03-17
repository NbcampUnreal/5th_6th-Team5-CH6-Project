#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmic_CY/InteractionBase.h"
#include "Components/TimelineComponent.h"
#include "SaveInterface.h"
#include "DoorBase.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UNavModifierComponent;

UCLASS()
class WARD_ZERO_API ADoorBase : public AActor, public IInteractionBase, public ISaveInterface
{
	GENERATED_BODY()
	
public:	
	ADoorBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

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
	virtual void PostActorCreated() override;
	virtual void HiddenActor() override;
	virtual FVector GetInteractionTargetLocation() const ;


	// ===== SaveInterface =====
	virtual FGuid GetActorID() const override;
	virtual void SaveActorState(class UWardSaveGame* SaveData) override;
	virtual void LoadActorState(class UWardSaveGame* SaveData) override;

protected:
	UPROPERTY(EditInstanceOnly)
	FGuid ActorID;

	UPROPERTY()
	bool bCollected = false;

	// Mesh
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Door;

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
};
