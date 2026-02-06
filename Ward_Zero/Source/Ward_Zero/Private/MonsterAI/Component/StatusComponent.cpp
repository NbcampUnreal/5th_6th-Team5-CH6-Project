// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAI/Component/StatusComponent.h"
#include "Public/MonsterAI/Data/MonsterDataAsset.h"

UStatusComponent::UStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStatusComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UStatusComponent::InitData(const UMonsterDataAsset* BaseData)
{
	if (BaseData)
	{
		MonsterData = BaseData;
		BaseSpeed = BaseData->BaseSpeed;
		ChaseSpeed = BaseData->ChaseSpeed;
		CurrentHP = BaseData->MaxHP;
	}
	bIsDataInit = true;
}

bool UStatusComponent::IsDataInit() const
{
	return bIsDataInit;
}


float UStatusComponent::GetBaseSpeed() const
{
	return BaseSpeed;
}

void UStatusComponent::SetBaseSpeed(float speed)
{
	BaseSpeed = speed;
}

float UStatusComponent::GetChaseSpeed() const
{
	return ChaseSpeed;
}

void UStatusComponent::SetChaseSpeed(float speed)
{
	ChaseSpeed = speed;
}

float UStatusComponent::GetArrivalRadius() const
{
	if (!MonsterData)
	{
		return 50.0f;
	}
	return MonsterData->ArrivalRadius;
}

float UStatusComponent::GetBaseDetectionRange() const
{
	if (!MonsterData)
	{
		return 1300.0f;
	}
	return MonsterData->BaseDetectionRange;
}

float UStatusComponent::GetViewAngle() const
{
	if (!MonsterData)
	{
		return 90.0f;
	}
	return MonsterData->ViewAngle;
}

float UStatusComponent::GetAttackRange() const
{
	if (!MonsterData)
	{
		return 100.0f;
	}
	return MonsterData->AttackRange;
}

float UStatusComponent::GetChaseRange() const
{
	if (!MonsterData)
	{
		return 1000.0f;
	}
	return MonsterData->ChaseRange;
}

float UStatusComponent::GetEyeHeight() const
{
	if (!MonsterData)
	{
		return 80.0f;
	}
	return MonsterData->EyeHeight;
}

float UStatusComponent::GetMaxLostTargetTime() const
{
	if (!MonsterData)
	{
		return 5.0f;
	}
	return MonsterData->MaxLostTargetTime;
}

float UStatusComponent::GetIdleSoundVolume() const
{
	if (!MonsterData)
	{
		return 1.0f;
	}
	return MonsterData->IdleSoundVolume;
}

float UStatusComponent::GetChaseSoundVolume() const
{
	if (!MonsterData)
	{
		return 1.0f;
	}
	return MonsterData->ChaseSoundVolume;
}



float UStatusComponent::GetHearingThreshold() const
{
	if (!MonsterData)
	{
		return 1.0f;
	}
	return MonsterData->HearingThreshold;
}

float UStatusComponent::GetLoseSightRange() const
{
	if (!MonsterData)
	{
		return 1500.0f;
	}
	return MonsterData->LoseSightRange;
}

float UStatusComponent::GetResistKnockdown() const
{
	if (!MonsterData)
	{
		return 0.f;
	}
	return MonsterData->ResistKnockdown;
}


float UStatusComponent::GetAttackDamage() const
{
	if (!MonsterData)
	{
		return 0.f;
	}
	return MonsterData->AttackDamage;
}

float UStatusComponent::GetHeadHitStunnedTime() const
{
	if (!MonsterData)
	{
		return 0.f;
	}
	return MonsterData->HeadHitStunnedTime;
}

float UStatusComponent::GetBodyHitStunnedTime() const
{
	if (!MonsterData)
	{
		return 0.f;
	}
	return MonsterData->BodyHitStunnedTime;
}

bool UStatusComponent::GetIsKnockdownSuperArmor() const
{
	if (!MonsterData)
	{
		return false;
	}
	return MonsterData->bIsKnockdownSuperArmor;
}

float UStatusComponent::GetWeakSpotDamageMultiplier() const
{
	if (!MonsterData)
	{
		return 2.0f;
	}
	return MonsterData->WeakSpotDamageMultiplier;
}

bool UStatusComponent::GetIsRecoveringCC() const
{
	return bIsRecoveringCC;
}

bool UStatusComponent::SetIsRecoveringCC(bool b)
{
	bIsRecoveringCC = b;
	return bIsRecoveringCC;
}

float UStatusComponent::ApplyDamage(float damage)
{
	CurrentHP = CurrentHP - damage;
	if (CurrentHP <= 0)
	{
		CurrentHP = 1.0f;
	}
	return CurrentHP;
}

float UStatusComponent::ApplyCriticalDamage(float damage)
{
	CurrentHP = CurrentHP - damage*GetWeakSpotDamageMultiplier();
	return CurrentHP;
}

bool UStatusComponent::GetIsDead() const
{
	return bIsDead;
}

void UStatusComponent::SetIsDead(bool b)
{
	bIsDead = b;
}
