#pragma once

#include "CoreMinimal.h"
#include "Gimmic_CY/Items/ItemBase.h"
#include "KeyCardActor.generated.h"

class UStaticMeshComponent;
UCLASS()
class WARD_ZERO_API AKeyCardActor : public AItemBase
{
	GENERATED_BODY()
	
public:
	AKeyCardActor();
	void BeginPlay() override;

	
	virtual void HandleInteraction_Implementation(APrototypeCharacter* Character) override;
	virtual bool CanBeInteracted_Implementation() const override { return true; }
	virtual EInteractionType GetInteractionType_Implementation() const override;
	
	UPROPERTY(EditAnywhere)
	TArray<AActor*> TargetActors;
	
};
