// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "BaseZombie_AIController.generated.h"

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
	
	/*FBlackboard::FKey TargetActorKeyID;
	FBlackboard::FKey IsStunnedKeyID;
	FBlackboard::FKey IsKnockdownKeyID;
	FBlackboard::FKey LastKnownLocationKeyID;
	FBlackboard::FKey InvestigateLocationKeyID;
	FBlackboard::FKey IsDeadKeyID;*/
	
protected:
	ABaseZombie_AIController();
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "AI")
	TObjectPtr<class UStatusComponent> StatusComp;
	
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
	
	
	UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	class UAIPerceptionComponent* AIPerceptionComp;
	
	
	virtual void Tick(float DeltaTime) override;
private:
	UPROPERTY(EditAnywhere)
	class UBehaviorTree* BT_BaseZombie;
	
};


