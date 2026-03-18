#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Base/DoorBase.h"
#include "DoubleDoor.generated.h"

UCLASS()
class WARD_ZERO_API ADoubleDoor : public ADoorBase
{
	GENERATED_BODY()
	
public:
	ADoubleDoor();
	
	virtual void OpenDoor() override;
	virtual void CloseDoor() override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RightDoor;

	// Interface
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;

	virtual void UpdateTimelineComp(float Output) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireKeyCard = true;


};
