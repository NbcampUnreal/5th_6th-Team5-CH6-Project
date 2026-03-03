// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MeleeAttackAbility.generated.h"

/**
 * 
 */
UCLASS()
class WARD_ZERO_API UMeleeAttackAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UMeleeAttackAbility();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "MeleeAttack")
	UAnimMontage* MeleeMontage;
};
