#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmic_CY/InteractionBase.h"
#include "SaveInterface.h"
#include "DoorBase.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class WARD_ZERO_API ADoorBase : public AActor, public IInteractionBase, public ISaveInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
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

};
