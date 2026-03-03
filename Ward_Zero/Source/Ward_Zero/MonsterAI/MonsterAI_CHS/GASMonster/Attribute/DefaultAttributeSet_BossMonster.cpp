// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultAttributeSet_BossMonster.h"
#include "GameplayEffectExtension.h"
#include "MonsterAI/MonsterAI_CHS/GASMonster/Character/GASBaseMonster.h"
#include "Net/UnrealNetwork.h"


UDefaultAttributeSet_BossMonster::UDefaultAttributeSet_BossMonster()
{
	InitMaxHealth(100.0f);
	InitHealth(100.0f);
}

void UDefaultAttributeSet_BossMonster::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetInComingDamageAttribute())
	{
		const float LocalInComingDamage = GetInComingDamage();
		
		if (LocalInComingDamage > 0.0f)
		{
			const float NewHealth = GetHealth() - LocalInComingDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
			if (GetHealth() <= 0.0f)
			{
				if (AGASBaseMonster* Enemy = Cast<AGASBaseMonster>(Data.Target.GetAvatarActor()))
				{
					//todo: enemy->die
				}
			}
		}
	}
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(),0.0f,GetMaxHealth()));
		if (GetHealth() <= 0.0f)
		{
			if (AGASBaseMonster* Enemy = Cast<AGASBaseMonster>(Data.Target.GetAvatarActor()))
			{
				//todo: Enemy->Die();
			}
		}
	}
	
	if (Data.EvaluatedData.Attribute == GetBaseSpeedAttribute())
	{
		SetBaseSpeed(FMath::Clamp(GetBaseSpeed(),0.0f,1000));
	}
}

void UDefaultAttributeSet_BossMonster::GetLifetimeReplicatedProps(
	TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UDefaultAttributeSet_BossMonster,Health,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDefaultAttributeSet_BossMonster,MaxHealth,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDefaultAttributeSet_BossMonster,BaseSpeed,COND_None,REPNOTIFY_Always);

}

void UDefaultAttributeSet_BossMonster::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDefaultAttributeSet_BossMonster,Health,OldHealth);

}

void UDefaultAttributeSet_BossMonster::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDefaultAttributeSet_BossMonster,Health,OldMaxHealth);

}

void UDefaultAttributeSet_BossMonster::OnRep_BaseSpeed(const FGameplayAttributeData& OldBaseSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDefaultAttributeSet_BossMonster,Health,OldBaseSpeed);

}
