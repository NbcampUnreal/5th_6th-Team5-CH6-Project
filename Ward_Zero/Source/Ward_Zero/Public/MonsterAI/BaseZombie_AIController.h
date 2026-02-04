// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseZombie_AIController.generated.h"

/**
 * 
 */
UCLASS()
class WARD_ZERO_API ABaseZombie_AIController : public AAIController
{
	GENERATED_BODY()

protected:
	ABaseZombie_AIController();
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
private:
	UPROPERTY(EditAnywhere)
	class UBehaviorTree* BT_BaseZombie;
	
};


