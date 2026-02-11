// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterAI/MonsterAI_CHS/Data/Type/MonsterStat.h"

#include "BaseZombie_AIController.generated.h"

struct FGameplayTag;
struct FAIStimulus;
/**
 * 
 */
UCLASS()
class WARD_ZERO_API ABaseZombie_AIController : public AAIController
{
	GENERATED_BODY()
public:
	void UpdatePerceptionConfig();

	
	UFUNCTION()
	void HandleInteractionRequest(FGameplayTag InteractingTag,const FVector& Destination, AActor* Interactor);
	
	
protected:
	ABaseZombie_AIController();
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "AI")
	TObjectPtr<class UStatusComponent> StatusComp;
	
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleMainStateChange(EMonsterMainState NewState);
	UFUNCTION()
	void HandleSubStateChange(EMonsterSubState NewState);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
	UFUNCTION()
	void StopInteracting();
	
	UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	class UAIPerceptionComponent* AIPerceptionComp;
	
	
	virtual void Tick(float DeltaTime) override;
private:
	UPROPERTY(EditAnywhere)
	class UBehaviorTree* BT_BaseZombie;
	
};


