#include "MonsterAI/Component/CombatComponent.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/DamageEvents.h"
#include "MonsterAI/AIController/BaseZombie_AIController.h"
#include "MonsterAI/AIController/WZAIKeys.h"
#include "MonsterAI/Component/StatusComponent.h"
#include "MonsterAI/Data/MonsterDataAsset.h"
#include "MonsterAI/Entity/BaseZombie.h"
#include "Weapon/WZDamageType.h"


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


void UCombatComponent::OnTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                    AActor* DamageCauser)
{
	if (!StatusComp)
	{
		return;
	}
	const UWZDamageType* WZDamageType = Cast<UWZDamageType>(DamageEvent.DamageTypeClass.GetDefaultObject());
	if (!WZDamageType)
	{
		return;
	}
	EHitDirection HitDir = EHitDirection::Front;
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageE = static_cast<const FPointDamageEvent*>(&DamageEvent);
		HitDir = GetHitDirection(PointDamageE->ShotDirection);
	}else if (DamageCauser)
	{
		FVector DirectionToCauser = (GetOwner()->GetActorLocation() - DamageCauser->GetActorLocation());
		HitDir = GetHitDirection(DirectionToCauser);
	}
	bool bIsCriticalHit = CheckCriticalHit(DamageEvent);
	if (bIsCriticalHit)
	{
		float WeaponKnockdownChance = WZDamageType->KnockdownProbability;
		float KnockdownChance = WeaponKnockdownChance * (1 - StatusComp->GetResistKnockdown());
		if (StatusComp->ApplyCriticalDamage(Damage) <= 0.0f)
		{
			OnDeath();
		}else if (FMath::RandRange(0.0f,100.0f) < KnockdownChance)
		{
			ApplyKnockdown(HitDir);
		}else
		{
			if (StatusComp->GetIsRecoveringCC() && StatusComp->GetIsKnockdownSuperArmor())
			{
				return;
			}
			ApplyStun(HitDir,bIsCriticalHit);
		}
	}else
	{
		
		StatusComp->ApplyDamage(Damage);
		if (StatusComp->GetIsRecoveringCC() && StatusComp->GetIsKnockdownSuperArmor())
		{
			return;
		}
		ApplyStun(HitDir,bIsCriticalHit);
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
	AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsAttacking,false);
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
	AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsStunned,true);
	AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsAttacking,false);
	UAnimMontage* MontageToPlay = nullptr;
	if (bIsCriticalHit)
	{
		switch (HitDir)
		{
		case EHitDirection::Front: MontageToPlay = MonsterData->CriticalHitReactMontages.Front; break;
		case EHitDirection::Back:  MontageToPlay = MonsterData->CriticalHitReactMontages.Back; break;
		case EHitDirection::Left:  MontageToPlay = MonsterData->CriticalHitReactMontages.Left; break;
		case EHitDirection::Right: MontageToPlay = MonsterData->CriticalHitReactMontages.Right; break;
		}
	}else
	{
		switch (HitDir)
		{
		case EHitDirection::Front: MontageToPlay = MonsterData->NormalHitReactMontages.Front; break;
		case EHitDirection::Back:  MontageToPlay = MonsterData->NormalHitReactMontages.Back; break;
		case EHitDirection::Left:  MontageToPlay = MonsterData->NormalHitReactMontages.Left; break;
		case EHitDirection::Right: MontageToPlay = MonsterData->NormalHitReactMontages.Right; break;
		}
	}
	

	if (MontageToPlay)
	{
		Owner->PlayAnimM(MontageToPlay);
		AIC->StopMovement(); 
	
		
	}
	
}


bool UCombatComponent::CheckCriticalHit(const FDamageEvent& DamageEvent)
{
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageE = static_cast<const FPointDamageEvent*>(&DamageEvent);
		FName HitBoneName = PointDamageE->HitInfo.BoneName;
		if (!MonsterData->WeakBoneName.IsNone() && HitBoneName == MonsterData->WeakBoneName)
		{
			return true;
		}
	}
	return false;
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
	UE_LOG(LogTemp,Warning,TEXT("CombatComp Attack Func start"))
	ABaseZombie* Owner = Cast<ABaseZombie>(GetOwner());
	if (!Owner) return;
	UAnimMontage* MontageToPlay = nullptr;
	MontageToPlay = MonsterData->AttackMontage;
	if (MontageToPlay)
	{
		UE_LOG(LogTemp,Warning,TEXT("CallAttack Montage"))
		Owner->PlayAnimM(MontageToPlay);
	}
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

