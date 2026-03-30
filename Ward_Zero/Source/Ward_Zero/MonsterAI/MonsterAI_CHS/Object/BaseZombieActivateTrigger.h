
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
	
	UFUNCTION(BlueprintCallable)
	void WakeUpZombies();

	FTimerHandle WaveTimerHandle;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* TriggerVolume;
	
	UPROPERTY(EditInstanceOnly, Category = "Setting")
	TArray<ABaseZombie*> Zombies;
	
	UPROPERTY(EditInstanceOnly, Category = "Setting")
	int ZombieNumPerOneWave = 2;
	
	UPROPERTY(EditInstanceOnly, Category = "Setting")
	float WaveInterval = 5.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Setting")
	int32 StageIndex = 0;
	
	int ZomIdx = 0;
	int ZombieNum = 0;
	bool bHasTriggered;
	
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	
	
public:
	UFUNCTION(BlueprintCallable)
	virtual void ActivateTrigger();
};
