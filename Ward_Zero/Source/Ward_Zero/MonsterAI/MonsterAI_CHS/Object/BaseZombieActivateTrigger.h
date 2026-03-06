
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseZombieActivateTrigger.generated.h"

class UBoxComponent;
class ABaseZombie;

UCLASS()
class WARD_ZERO_API ABaseZombieActivateTrigger : public AActor
{
	GENERATED_BODY()

public:
	ABaseZombieActivateTrigger();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	UBoxComponent* TriggerVolume;
	
	UPROPERTY(EditInstanceOnly, Category = "Trigger Setup")
	TArray<ABaseZombie*> Zombies;
	
	bool bHasTriggered;
	
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
public:
	
};
