#include "MonsterAI/MonsterAI_CHS/Component/CombatComponent.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/DamageEvents.h"
#include "GameplayTagContainer.h"
#include "MonsterAI/MonsterAI_CHS/Data/Type/GameTypes.h"
#include "MonsterAI/MonsterAI_CHS/AI/BaseZombie_AIController.h"
#include "MonsterAI/MonsterAI_CHS/AI/WZAIKeys.h"
#include "MonsterAI/MonsterAI_CHS/Component/StatusComponent.h"
#include "MonsterAI/MonsterAI_CHS/Data/MonsterDataAsset.h"
#include "MonsterAI/MonsterAI_CHS/Entity/BaseZombie.h"
#include "MonsterAI/MonsterAI_CHS/Physics/TagPhysicalMaterial.h"
#include "MonsterAI/MonsterAI_CHS/Weapon/WZDamageType.h"
#include "Kismet/GameplayStatics.h"


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
	if (StatusComp->GetIsRecoveringCC())
	{
		return;
	}
	
	
	UAnimMontage* MontageToPlay = nullptr;
	StatusComp->SetSubState(EMonsterSubState::Knockdown);
	
	if (GetIsAttacking())
	{
		Owner->StopAnimMontage();
		SetIsAttacking(false);
	}
	
	switch (HitDir)
	{
		case EHitDirection::Front: MontageToPlay = MonsterData->KnockdownMontages.Front; break;
		case EHitDirection::Back: MontageToPlay = MonsterData->KnockdownMontages.Back; break;
		case EHitDirection::Left: MontageToPlay = MonsterData->KnockdownMontages.Left; break;
		case EHitDirection::Right: MontageToPlay = MonsterData->KnockdownMontages.Right; break;

	}
	
	
	if (MontageToPlay)
	{
		StatusComp->SetIsRecoveringCC(true);
		AIC->StopMovement();
		if (AIC->GetBlackboardComponent())
		{
			AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsKnockedDown,true);
			AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsStunned,false);
		}
		
		Owner->PlayAnimM(MontageToPlay);
	}
	
	//Ragdoll
	/*ABaseZombie* Owner = Cast<ABaseZombie>(GetOwner());
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
	*/
	
	
}

void UCombatComponent::ApplyStun(EHitDirection HitDir, EHitPart HitPart)
{
	/*if (StatusComp->GetIsRecoveringCC() && StatusComp->GetIsKnockdownSuperArmor())
	{
		return;
	}*/
	
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
	if (StatusComp)
	{
		StatusComp->SetSubState(EMonsterSubState::Stun);
	}
	if (GetIsAttacking())
	{
		Owner->StopAnimMontage();
		SetIsAttacking(false);
	}
	if (HitPart == EHitPart::Head)
	{
		if (StatusComp->GetIsRecoveringCC())
		{
			return;
		}
		switch (HitDir)
		{
		case EHitDirection::Front: MontageToPlay = MonsterData->CriticalHitReactMontages.Front; break;
		case EHitDirection::Back: MontageToPlay = MonsterData->CriticalHitReactMontages.Back; break;
		case EHitDirection::Left: MontageToPlay = MonsterData->CriticalHitReactMontages.Left; break;
		case EHitDirection::Right: MontageToPlay = MonsterData->CriticalHitReactMontages.Right; break;

		}
	}else if (HitPart == EHitPart::Body)
	{

		switch (HitDir)
		{
		case EHitDirection::Front: MontageToPlay = MonsterData->NormalHitReactMontages.Front; break;
		case EHitDirection::Back:  MontageToPlay = MonsterData->NormalHitReactMontages.Back; break;
		case EHitDirection::Left:  MontageToPlay = MonsterData->NormalHitReactMontages.Left; break;
		case EHitDirection::Right: MontageToPlay = MonsterData->NormalHitReactMontages.Right; break;
		}
	}else
	{

		if (StatusComp->GetIsRecoveringCC())
		{
			//UE_LOG(LogTemp,Warning,TEXT("Recoveringcc is true"));
			return;
		}
		StatusComp->SetIsRecoveringCC(true);
		AIC->StopMovement();
		if (AIC->GetBlackboardComponent())
		{
			AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsLegFalling,true);

		}
		if (HitPart == EHitPart::LegLeft)
		{
			MontageToPlay = MonsterData->LegHitReactionMontage.LeftLegHitReaction;
		}else if (HitPart == EHitPart::LegRight)
		{
			MontageToPlay = MonsterData->LegHitReactionMontage.RightLegHitReaction;
		}
	}
	
	
	
	

	if (MontageToPlay)
	{
		if (AIC->GetBlackboardComponent())
		{
			AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsStunned,true);

		}
		Owner->PlayAnimM(MontageToPlay);
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
	SetIsAttacking(true);
	if (StatusComp)
	{
		StatusComp->SetSubState(EMonsterSubState::Attack);
	}
	ABaseZombie* Owner = Cast<ABaseZombie>(GetOwner());
	if (!Owner) return;
	UAnimMontage* MontageToPlay = nullptr;
	if (MonsterData->AttackMontages.Num() != 0)
	{
		int32 Randindex = FMath::RandRange(0,100) % MonsterData->AttackMontages.Num();
		MontageToPlay = MonsterData->AttackMontages[Randindex];
	}
	if (MontageToPlay)
	{
		//UE_LOG(LogTemp,Warning,TEXT("CallAttack Montage"))
		Owner->PlayAnimM(MontageToPlay);
	}
}

bool UCombatComponent::GetIsAttacking()
{
	return bIsAttacking;
}

void UCombatComponent::SetIsAttacking(bool isAttacking)
{
	bIsAttacking = isAttacking;
}

void UCombatComponent::SpawnHitEffect(FDamageEvent const& DamageEvent)
{
	const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
	FHitResult HitInfo = PointDamageEvent->HitInfo;
	if (MonsterData)
	{
		if (MonsterData->BloodEffectSystem)
		{
			FRotator BloodRotation = HitInfo.ImpactNormal.Rotation();

			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MonsterData->BloodEffectSystem,
				HitInfo.ImpactPoint, 
				BloodRotation        
			);
		}
		if (MonsterData->BulletHitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, MonsterData->BulletHitSound, HitInfo.ImpactPoint);
		}
	}
}

void UCombatComponent::ProcessDamageLogic(float Damage, EHitPart HitPart, const FVector& AttackDir,
                                          const UWZDamageType* DamageType, AActor* DamageCauser)
{
	EHitDirection HitDir = GetHitDirection(AttackDir);
	
	if (HitPart == EHitPart::Head)
	{
		float KnockdownChance = DamageType->KnockdownProbability * (1 - StatusComp->GetResistKnockdown());
		if (StatusComp->ApplyDamage(Damage,true) <= 0.0f)
		{
			UE_LOG(LogTemp,Warning,TEXT("Death"));
			OnDeath();
		}else if (FMath::RandRange(0.0f,100.0f) < KnockdownChance)
		{
			UE_LOG(LogTemp,Warning,TEXT("Knockdown"));
			ApplyKnockdown(HitDir);
		}else
		{
			ApplyStun(HitDir, HitPart);
		}
	}else
	{
		if (StatusComp->ApplyDamage(Damage,false) <= 0.0f)
		{
			OnDeath();
		}
		if (StatusComp->GetIsRecoveringCC() && StatusComp->GetIsKnockdownSuperArmor())
		{
			return;
		}
		ApplyStun(HitDir, HitPart);
	}
	
}

void UCombatComponent::HandleAllDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                       AActor* DamageCauser)
{
	const UWZDamageType* WZDamageType = Cast<UWZDamageType>(DamageEvent.DamageTypeClass.GetDefaultObject());
	
	FVector HitDir = FVector::ZeroVector;
	EHitPart HitPart = EHitPart::Body;
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		StatusComp->SetMainState(EMonsterMainState::Combat);
		
		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
		FHitResult HitInfo = PointDamageEvent->HitInfo;
		
		SpawnHitEffect(DamageEvent);
		
		
		if (UTagPhysicalMaterial* PM = Cast<UTagPhysicalMaterial>(PointDamageEvent->HitInfo.PhysMaterial.Get()))
		{
			FGameplayTag HitTag = PM->HitPartTag;
			if (HitTag.MatchesTag(FGameplayTag::RequestGameplayTag(GPTags::Head)))
			{
				UE_LOG(LogTemp,Warning,TEXT("Hit Tag Head"))
				HitPart = EHitPart::Head;
			}else if (HitTag.MatchesTag(FGameplayTag::RequestGameplayTag(GPTags::LegLeft)))
			{
				UE_LOG(LogTemp,Warning,TEXT("Hit Tag Letf Leg"))

				HitPart = EHitPart::LegLeft;
			}else if (HitTag.MatchesTag(FGameplayTag::RequestGameplayTag(GPTags::LegRight)))
			{
				UE_LOG(LogTemp,Warning,TEXT("Hit Tag Right Leg"))

				HitPart = EHitPart::LegRight;
			}
		}
		HitDir = PointDamageEvent->ShotDirection;
	}
	else
	{
		if (DamageCauser)
		{
			HitDir = GetOwner()->GetActorLocation() - DamageCauser->GetActorLocation();
		}
	}

	ProcessDamageLogic(Damage, HitPart, HitDir, WZDamageType, DamageCauser);
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

