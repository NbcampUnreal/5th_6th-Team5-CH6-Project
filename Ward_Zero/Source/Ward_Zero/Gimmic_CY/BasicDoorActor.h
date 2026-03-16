#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/DoorBase.h"
#include "BasicDoorActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class WARD_ZERO_API ABasicDoorActor : public ADoorBase
{
	GENERATED_BODY()
	
public:
	ABasicDoorActor();

	virtual bool CanBeInteracted_Implementation() const override { return true; }
	virtual EInteractionType GetInteractionType_Implementation() const override;

protected:

	// Door Mesh
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Door;
};
