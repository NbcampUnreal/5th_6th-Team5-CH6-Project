// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "WZDamageType.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EDamageSource : uint8
{
	None        UMETA(DisplayName = "None"),
	Melee       UMETA(DisplayName = "Melee"),
	Gun			UMETA(DisplayName = "Gun"),
	Explosion   UMETA(DisplayName = "Explosion"),
	BossRush	UMETA(DisplayName = "BossRush")
};
UCLASS()
class WARD_ZERO_API UWZDamageType : public UDamageType
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Damage Type Info")
	EDamageSource DamageSource = EDamageSource::None;
	
	UPROPERTY(EditDefaultsOnly)
	float StunProbability = 100.f;
	
	UPROPERTY(EditDefaultsOnly)
	float KnockdownProbability = 0.f;
	
	UPROPERTY(EditDefaultsOnly)
	bool bRequiresHeadshotForStun = false;
	
	UPROPERTY(EditDefaultsOnly)
	bool bForceStun = false;
};
