// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAI/MonsterAI_CHS/Component/StatusComponent.h"
#include "MonsterAI/MonsterAI_CHS/Data/MonsterDataAsset.h"

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
		if (BaseData->StateConfigMap.Find(EMonsterMainState::Idle) != nullptr && BaseData->StateConfigMap.Find(EMonsterMainState::Combat) != nullptr)
		{
			Speed = FSpeed(BaseData->StateConfigMap[EMonsterMainState::Idle].MovementSpeed,BaseData->StateConfigMap[EMonsterMainState::Combat].MovementSpeed);
		}
		Health = FHealth(BaseData->MaxHP,BaseData->MaxHP);
	}
	bIsDataInit = true;
}

bool UStatusComponent::IsDataInit() const
{
	return bIsDataInit;
}

void UStatusComponent::SetMainState(EMonsterMainState NewState)
{
	if (MainState == NewState)
	{
		return;
	}
	MainState = NewState;
	OnMainStateChanged.Broadcast(NewState);
}

void UStatusComponent::SetSubState(EMonsterSubState NewState)
{
	if (SubState == NewState) return;
	switch (NewState)
	{
	case EMonsterSubState::Stun:
	case EMonsterSubState::Attack:
	case EMonsterSubState::Chase:
	case EMonsterSubState::Knockdown:
		SetMainState(EMonsterMainState::Combat);
		break;
	default:
		break;
	}
	SubState = NewState;
	OnSubStateChanged.Broadcast(SubState);
}

EMonsterMainState UStatusComponent::GetMainState() const
{
	return MainState;
}

EMonsterSubState UStatusComponent::GetSubState() const
{
	return SubState;
}

EMonsterMainState UStatusComponent::GetStartState() const
{
	return StartState;
}


float UStatusComponent::GetBaseSpeed() const
{
	return Speed.GetBaseSpeed();
}

void UStatusComponent::SetBaseSpeed(float speed)
{
	Speed.SetBaseSpeed(speed);
}

float UStatusComponent::GetChaseSpeed() const
{
	return Speed.GetChaseSpeed();
}

void UStatusComponent::SetChaseSpeed(float speed)
{
	Speed.SetChaseSpeed(speed);
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

/*float UStatusComponent::GetChaseRange() const
{
	if (!MonsterData)
	{
		return 1000.0f;
	}
	return MonsterData->ChaseRange;
}*/

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
	return MonsterData->StateConfigMap[EMonsterMainState::Idle].LoopSoundVolume;
}

float UStatusComponent::GetChaseSoundVolume() const
{
	if (!MonsterData)
	{
		return 1.0f;
	}
	return MonsterData->StateConfigMap[EMonsterMainState::Combat].LoopSoundVolume;
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

float UStatusComponent::GetCurrentHP() const
{
	return Health.GetCurrentHP();
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

/*FName UStatusComponent::GetWeakBoneName() const
{
	if (!MonsterData)
	{
		return "None";
	}
	return MonsterData->WeakBoneName;
}

FName UStatusComponent::GetLeftBoneName() const
{
	if (!MonsterData)
	{
		return "None";
	}
	return MonsterData->LeftLegBoneName;
}

FName UStatusComponent::GetRightBoneName() const
{
	if (!MonsterData)
	{
		return "None";
	}
	return MonsterData->RightLegBoneName;
}*/

float UStatusComponent::ApplyDamage(float Amount, bool bIsCritical)
{
	return Health.AppyDamage(Amount,bIsCritical);
}

bool UStatusComponent::GetIsDead() const
{
	return bIsDead;
}

void UStatusComponent::SetIsDead(bool b)
{
	bIsDead = b;
}
