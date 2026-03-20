// SaveActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gimmic_CY/Base/InteractionBase.h"
#include "SaveActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class WARD_ZERO_API ASaveActor : public AActor, public IInteractionBase
{
	GENERATED_BODY()

public:
	ASaveActor();

	// ===== IInteractionBase 구현 =====
	virtual void OnIneracted_Implementation(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted_Implementation() const override { return true; }
	virtual EInteractionType GetInteractionType_Implementation() const override;
	virtual void OnIneractionRangeEntered_Implementation() override {}
	virtual void OnIneractionRangeExited_Implementation() override {}
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override {}
	virtual bool SetBCanInteract(bool IsCanInteract) override;
	virtual bool GetBCanInteract() const override;
	virtual void SaveActorState() const override;
	//virtual void HiddenActor() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	UStaticMeshComponent* SaveMesh;
};
