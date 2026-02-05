// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "WZDamageType.generated.h"

/**
 * 
 */
UCLASS()
class WARD_ZERO_API UWZDamageType : public UDamageType
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	float StunProbability = 100.f;
	
	UPROPERTY(EditDefaultsOnly)
	float KnockdownProbability = 0.f;
	
	UPROPERTY(EditDefaultsOnly)
	bool bRequiresHeadshotForStun = false;
	
	UPROPERTY(EditDefaultsOnly)
	bool bForceStun = false;
};
