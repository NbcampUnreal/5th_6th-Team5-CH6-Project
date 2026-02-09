#include "MonsterAI/MonsterAI_CHS/Component/CombatComponent.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/DamageEvents.h"
#include "MonsterAI/MonsterAI_CHS/Data/Type/GameTypes.h"
#include "MonsterAI/MonsterAI_CHS/AI/BaseZombie_AIController.h"
#include "MonsterAI/MonsterAI_CHS/AI/WZAIKeys.h"
#include "MonsterAI/MonsterAI_CHS/Component/StatusComponent.h"
#include "MonsterAI/MonsterAI_CHS/Data/MonsterDataAsset.h"
#include "MonsterAI/MonsterAI_CHS/Entity/BaseZombie.h"
#include "MonsterAI/MonsterAI_CHS/Weapon/WZDamageType.h"


// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	StatusComp = GetOwner()->FindComponentByClass<UStatusComponent>();
	if (ABaseZombie* Zombie = Cast<ABaseZombie>(GetOwner()))
	{
		MonsterData = Zombie->GetMonsterData();
	}
	
}
void UCombatComponent::ApplyKnockdown(EHitDirection HitDir)
{
	ABaseZombie* Owner = Cast<ABaseZombie>(GetOwner());
	if (!Owner)
	{
		return;
	}
	ABaseZombie_AIController* AIC = Cast<ABaseZombie_AIController>(Owner->GetController());
	if (!AIC)
	{
		return;
	}
	AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsKnockedDown,true);
	AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsStunned,false);
	Owner->StartRagdollKnockdown(HitDir);
	
	
}

void UCombatComponent::ApplyStun(EHitDirection HitDir, bool bIsCriticalHit)
{
	if (StatusComp->GetIsRecoveringCC() && StatusComp->GetIsKnockdownSuperArmor())
	{
		return;
	}
	ABaseZombie* Owner = Cast<ABaseZombie>(GetOwner());
	if (!Owner)
	{
		return;
	}
	ABaseZombie_AIController* AIC = Cast<ABaseZombie_AIController>(Owner->GetController());
	if (!AIC)
	{
		return;
	}
	UAnimMontage* MontageToPlay = nullptr;
	
	
	switch (HitDir)
	{
		case EHitDirection::Front: MontageToPlay = MonsterData->NormalHitReactMontages.Front; break;
		case EHitDirection::Back:  MontageToPlay = MonsterData->NormalHitReactMontages.Back; break;
		case EHitDirection::Left:  MontageToPlay = MonsterData->NormalHitReactMontages.Left; break;
		case EHitDirection::Right: MontageToPlay = MonsterData->NormalHitReactMontages.Right; break;
	}
	
	

	if (MontageToPlay)
	{
		AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsStunned,true);
		Owner->PlayAnimM(MontageToPlay);
		AIC->StopMovement(); 
		
	}
	
}


void UCombatComponent::OnDeath()
{
	if (StatusComp && StatusComp->GetIsDead())
	{
		return;
	}
	if (ABaseZombie* Owner = Cast<ABaseZombie>(GetOwner()))
	{
		Owner->OnDeath();
	}
}

void UCombatComponent::Attack()
{
	//UE_LOG(LogTemp,Warning,TEXT("CombatComp Attack Func start"))
	ABaseZombie* Owner = Cast<ABaseZombie>(GetOwner());
	if (!Owner) return;
	UAnimMontage* MontageToPlay = nullptr;
	MontageToPlay = MonsterData->AttackMontage;
	if (MontageToPlay)
	{
		//UE_LOG(LogTemp,Warning,TEXT("CallAttack Montage"))
		Owner->PlayAnimM(MontageToPlay);
	}
}

void UCombatComponent::ProcessDamageLogic(float Damage, FName HitBone, const FVector& AttackDir,
	const UWZDamageType* DamageType, AActor* DamageCauser)
{
	EHitDirection HitDir = EHitDirection::Front;
	if (DamageType->DamageSource == EDamageSource::Gun)
	{
		HitDir = GetHitDirection(AttackDir);
		float KnockdownChance = DamageType->KnockdownProbability * (1 - StatusComp->GetResistKnockdown());
		bool bIsCriticalHit =  (HitBone == StatusComp->GetWeakBoneName());
		UE_LOG(LogTemp,Warning,TEXT("Hit! CurrentHP: %f, HitBone: %s, WeakBone: %s, bIsCritical: %hs"),StatusComp->GetCurrentHP(),*HitBone.ToString(),*StatusComp->GetWeakBoneName().ToString(), bIsCriticalHit ? "true" : "false");
		if (bIsCriticalHit)
		{
			UE_LOG(LogTemp,Warning,TEXT("Critical"));
			if (StatusComp->ApplyDamage(Damage,bIsCriticalHit) <= 0.0f)
			{
				UE_LOG(LogTemp,Warning,TEXT("Death"));
				OnDeath();
			}else if (FMath::RandRange(0.0f,100.0f) < KnockdownChance)
			{
				UE_LOG(LogTemp,Warning,TEXT("Knockdown"));
				ApplyKnockdown(HitDir);
			}
		}else
		{
			StatusComp->ApplyDamage(Damage,bIsCriticalHit);
			if (StatusComp->GetIsRecoveringCC() && StatusComp->GetIsKnockdownSuperArmor())
			{
				return;
			}
			ApplyStun(HitDir,bIsCriticalHit);
		}
	}else
	{
		StatusComp->ApplyDamage(Damage,false);
		if (StatusComp->GetIsRecoveringCC() && StatusComp->GetIsKnockdownSuperArmor())
		{
			return;
		}
		ApplyStun(HitDir,false);
	}
	
}

void UCombatComponent::HandleAllDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                       AActor* DamageCauser)
{
	const UWZDamageType* WZDamageType = Cast<UWZDamageType>(DamageEvent.DamageTypeClass.GetDefaultObject());
    
	FName HitBone = NAME_None;
	FVector HitDir = FVector::ZeroVector;

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
		HitBone = PointDamageEvent->HitInfo.BoneName;
		HitDir = PointDamageEvent->ShotDirection;
	}
	else
	{
		if (DamageCauser)
		{
			HitDir = GetOwner()->GetActorLocation() - DamageCauser->GetActorLocation();
		}
	}

	ProcessDamageLogic(Damage, HitBone, HitDir, WZDamageType, DamageCauser);
}

EHitDirection UCombatComponent::GetHitDirection(const FVector& ShotDirection)
{
	FVector OwnerForward = GetOwner()->GetActorForwardVector();
	FVector OwnerRight = GetOwner()->GetActorRightVector();
	
	FVector ToAttacker = -ShotDirection;
	ToAttacker.Z = 0.0f;
	ToAttacker.Normalize();
	
	float ForwardDot = FVector::DotProduct(OwnerForward, ToAttacker);
	float RightDot = FVector::DotProduct(OwnerRight, ToAttacker);
	
	if (ForwardDot >= 0.5f)
	{
		return EHitDirection::Front;
	}else if (ForwardDot <= -0.5f)
	{
		return EHitDirection::Back;
	}else if (RightDot > 0.0f)
	{
		return EHitDirection::Right;
	}else
	{
		return EHitDirection::Left;
	}
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

