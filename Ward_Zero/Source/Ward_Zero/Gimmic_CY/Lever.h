#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Base/ObjectBase.h"
#include "Lever.generated.h"

class ASingleDoor;

UCLASS()
class WARD_ZERO_API ALever : public AObjectBase
{
	GENERATED_BODY()
	
public:
	ALever();

private:
	void LeverOpenDoor();
	void LeverCloseDoor();
	void LeverLockInteraction();
	void LeverUnLockInteraction();


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ASingleDoor*> DoorsForOpen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ASingleDoor*> DoorsForClose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> InteractionActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> UnInteractionActors;

public:

	void ActivateLever();
	// ===== IGimmickInterface =====
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
};
