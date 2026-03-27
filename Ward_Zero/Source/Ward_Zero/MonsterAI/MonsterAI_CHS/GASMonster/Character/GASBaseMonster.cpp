// Fill out your copyright notice in the Description page of Project Settings.


#include "GASBaseMonster.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"


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

void AGASBaseMonster::Die()
{
	FTimerHandle TimerHandle;
    
	TWeakObjectPtr<AGASBaseMonster> WeakThis(this);
    
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([WeakThis]()
	{
		if (WeakThis.IsValid() && WeakThis->GetMesh() && WeakThis->AbilitySystemComponent)
		{
			WeakThis->AbilitySystemComponent->CancelAbilities();
			WeakThis->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WeakThis->GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
			WeakThis->GetMesh()->SetSimulatePhysics(true);
		}
	}), 2.0f, false);
	
	OnDeathDelegate.Broadcast(this);
	
}

void AGASBaseMonster::BeginPlay()
{
	Super::BeginPlay();
	
	
}

float AGASBaseMonster::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
		FHitResult HitInfo = PointDamageEvent->HitInfo;
		if (BloodEffectSystem)
		{
			FRotator BloodRotation = HitInfo.ImpactNormal.Rotation();

			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				BloodEffectSystem,
				HitInfo.ImpactPoint, 
				BloodRotation        
			);
		}
	}
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
		
	if (ActualDamage > 0.f && AbilitySystemComponent && ReceiveDamageEffect)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddInstigator(EventInstigator, DamageCauser);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(ReceiveDamageEffect, 1.0f, Context);

		if (SpecHandle.IsValid())
		{
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageTag, ActualDamage);
            
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

		
	
	return ActualDamage;
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
	OnHealthChanged.Broadcast(Data.NewValue,DefaultAttributeSet_BossMonster->GetMaxHealth());
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

