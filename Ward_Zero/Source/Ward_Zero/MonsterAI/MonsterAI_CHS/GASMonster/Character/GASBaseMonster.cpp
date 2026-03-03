// Fill out your copyright notice in the Description page of Project Settings.


#include "GASBaseMonster.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


AGASBaseMonster::AGASBaseMonster()
{
	PrimaryActorTick.bCanEverTick = false;
	
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UDefaultAttributeSet_BossMonster::GetBaseSpeedAttribute()).AddUObject(this,&AGASBaseMonster::OnMoveSpeedChanged);
	DefaultAttributeSet_BossMonster = CreateDefaultSubobject<UDefaultAttributeSet_BossMonster>(FName("DefaultAttributeSet_BossMonster"));
}

class UAbilitySystemComponent* AGASBaseMonster::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGASBaseMonster::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void AGASBaseMonster::InitializeAttributes()
{
	if (AbilitySystemComponent && DefaultAttributeEffect)
	{
		FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect,1.0f,ContextHandle);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void AGASBaseMonster::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this,this);
		if (HasAuthority())
		{
			InitializeAttributes();
			for (const TSubclassOf<UGameplayAbility>& AbilityClass: DefaultAbilities)
			{
				if (AbilityClass)
				{
					FGameplayAbilitySpec Spec(AbilityClass,1,-1,this);
					AbilitySystemComponent->GiveAbility(Spec);
				}
			}
			
		}
		
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(DefaultAttributeSet_BossMonster->GetHealthAttribute())
		.AddUObject(this, &AGASBaseMonster::OnHealthChangedCallback);
		
	}
}





void AGASBaseMonster::OnHealthChangedCallback(const FOnAttributeChangeData& Data) const
{
}

void AGASBaseMonster::OnStaminaChangedCallback(const FOnAttributeChangeData& Data) const
{
}

void AGASBaseMonster::OnMoveSpeedChanged(const FOnAttributeChangeData& Data) const
{
	UpdateMoveSpeed(Data.NewValue);
}

void AGASBaseMonster::UpdateMoveSpeed(float NewSpeed) const
{
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = NewSpeed;
	}
}

