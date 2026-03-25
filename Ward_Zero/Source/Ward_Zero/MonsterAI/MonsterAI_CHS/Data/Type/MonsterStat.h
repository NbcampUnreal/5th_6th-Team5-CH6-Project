#pragma once
#include "CoreMinimal.h"
#include "MonsterStat.generated.h"

UENUM(BlueprintType)
enum class EMonsterMainState:uint8
{
	Idle,
	Patrol,
	Investigate,
	Interacting,
	Combat,
	Dead
};

UENUM(BlueprintType)
enum class EMonsterSubState: uint8
{
	None,
	Knockdown,
	Stun,
	Attack,
	Chase
};

USTRUCT(BlueprintType)
struct FMonsterStateSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	float MaxMovementSpeed = 300.f;
	
	UPROPERTY(EditAnywhere)
	float MinMovementSpeed = 150.f;
	
	UPROPERTY(EditAnywhere)
	class USoundBase* StateLoopSound = nullptr;

	UPROPERTY(EditAnywhere)
	float LoopSoundVolume = 0.f;
	
	UPROPERTY(EditAnywhere)
	float YawRotateSpeed = 180.f;
	
};


class FHealth
{
public:
	FHealth() : CurrentHP(0.f), MaxHP(0.f) {}
	FHealth(float InCurrent, float InMax): CurrentHP(FMath::Clamp(InCurrent,0.f,InMax)), MaxHP(InMax) {}
	
	float AppyDamage(float Amount, bool bIsCritical)
	{
		if (true)
		{
			CurrentHP = FMath::Max(0.f, CurrentHP - Amount);
		}else
		{
			CurrentHP = FMath::Max(1.f, CurrentHP - Amount);
		}
		return CurrentHP;
	}
	float GetCurrentHP() const { return CurrentHP; }
private:
	float CurrentHP;
	float MaxHP;
};


class FSpeed
{
public:
	FSpeed() :BaseSpeed(0.f),ChaseSpeed(0.f) {}
	FSpeed(float InBaseSpeed, float InChaseSpeed): BaseSpeed(FMath::Clamp(InBaseSpeed,0.f,InBaseSpeed))
	,ChaseSpeed(FMath::Clamp(InChaseSpeed,0.f,InChaseSpeed)) {}
	
	float GetBaseSpeed() const { return BaseSpeed; }
	void SetBaseSpeed(float InBaseSpeed) { BaseSpeed = InBaseSpeed; }
	float GetChaseSpeed() const { return ChaseSpeed; }
	void SetChaseSpeed(float InChaseSpeed) { ChaseSpeed = InChaseSpeed; }
	
private:
	float BaseSpeed;
	float ChaseSpeed;
};
