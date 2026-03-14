#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmic_CY/InteractionBase.h"
#include "Lever.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class ADoorActor;

UCLASS()
class WARD_ZERO_API ALever : public AActor, public IInteractionBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALever();

protected:
	virtual void BeginPlay() override;

protected:
	// Root Scene
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* InteractionBox;

	// Lever Mesh
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Lever;

private: 
	void LeverOpenDoor();
	void LeverCloseDoor();
	void LeverLockInteraction();
	void LeverUnLockInteraction();

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
	virtual void HiddenActor() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ADoorActor*> DoorsForOpen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ADoorActor*> DoorsForClose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> InteractionActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> UnInteractionActors;
	
public:

	void ActivateLever();
};
