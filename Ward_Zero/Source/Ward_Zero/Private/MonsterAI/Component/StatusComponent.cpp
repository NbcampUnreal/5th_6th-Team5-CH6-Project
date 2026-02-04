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
	return MonsterData->ArrivalRadius;
}

float UStatusComponent::GetBaseDetectionRange() const
{
	return MonsterData->BaseDetectionRange;
}

float UStatusComponent::GetViewAngle() const
{
	return MonsterData->ViewAngle;
}

float UStatusComponent::GetAttackRange() const
{
	return MonsterData->AttackRange;
}

float UStatusComponent::GetChaseRange() const
{
	return MonsterData->ChaseRange;
}

float UStatusComponent::GetEyeHeight() const
{
	return MonsterData->EyeHeight;
}

float UStatusComponent::GetMaxLostTargetTime() const
{
	return MonsterData->MaxLostTargetTime;
}

float UStatusComponent::GetIdleSoundVolume() const
{
	return MonsterData->IdleSoundVolume;
}

float UStatusComponent::GetChaseSoundVolume() const
{
	return MonsterData->ChaseSoundVolume;
}


float UStatusComponent::GetStunTime() const
{
	return MonsterData->StunnedTime;
}

float UStatusComponent::GetHearingThreshold() const
{
	return MonsterData->HearingThreshold;
}