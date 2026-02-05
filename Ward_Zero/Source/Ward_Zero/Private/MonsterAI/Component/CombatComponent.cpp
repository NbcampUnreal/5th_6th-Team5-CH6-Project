#include "MonsterAI/Component/CombatComponent.h"

#include "Engine/DamageEvents.h"
#include "MonsterAI/Component/StatusComponent.h"
#include "Weapon/WZDamageType.h"


// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UCombatComponent::OnTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if (!StatusComp)
	{
		return;
	}
	/*if (StatusComp->ApplyDamage(Damage) <= 0.0f)
	{
		OnDeath();
		return;
	}*/
	if (bIsResistingCC)
	{
		return;
	}
	const UWZDamageType* WZDamageType = Cast<UWZDamageType>(DamageEvent.DamageTypeClass.GetDefaultObject());
	if (!WZDamageType)
	{
		return;
	}
	//bool bIsHeadShot = CheckHeadShot(DamageEvent);
	
	float WeaponStunChance = WZDamageType->KnockdownProbability;
	
	float KnockdownChance = WeaponStunChance * (1 - StatusComp->GetResistStun());
	if (FMath::RandRange(0.0f,100.0f) < KnockdownChance)
	{
		//ApplyKnockdown(true);
		return;
	}
	
	
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	StatusComp = GetOwner()->FindComponentByClass<UStatusComponent>();
	
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

