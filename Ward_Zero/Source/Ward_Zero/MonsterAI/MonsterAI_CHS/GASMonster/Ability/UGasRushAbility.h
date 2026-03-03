// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "UGasRushAbility.generated.h"

/**
 * 
 */
UCLASS()
class WARD_ZERO_API UUGasRushAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UUGasRushAbility();
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Rush")
	UAnimMontage* RushMontage;
};
