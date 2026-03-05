#include "Character/Components/PlayerStatusComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

UPlayerStatusComponent::UPlayerStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerStatusComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrHealth = MaxHealth;
	bIsDead = false;

	CurrStamina = MaxStamina;
	bIsExhausted = false;
}

void UPlayerStatusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDead) return;

	APrototypeCharacter* Player = Cast<APrototypeCharacter>(GetOwner());
	if (!Player) return;

	if (Player->GetIsRunning())
	{		
		CurrStamina = FMath::Clamp(CurrStamina - (StaminaDrainRate * DeltaTime), 0.0f, MaxStamina);

		if (CurrStamina <= 0.0f)
		{
			bIsExhausted = true;
		}
	}
	else
	{
		CurrStamina = FMath::Clamp(CurrStamina + (StaminaRegenRate * DeltaTime), 0.0f, MaxStamina);

		if (bIsExhausted && CurrStamina >= MinStaminaToSprint)
		{
			bIsExhausted = false;
		}
	}
}

float UPlayerStatusComponent::ApplyDamage(float DamageAmount)
{
	if (bIsDead) return 0.0f;

	float ActualDamage = FMath::Min(CurrHealth, DamageAmount);
	CurrHealth -= ActualDamage;

	UE_LOG(LogTemp, Warning, TEXT("[Player] HP: %.1f / %.1f"), CurrHealth, MaxHealth);

	if (CurrHealth <= 0.0f)
	{
		CurrHealth = 0.0f;
		bIsDead = true;
		OnPlayerDied.Broadcast(); //캐릭터 Die 알림 
		OnHealthChanged.Broadcast(CurrHealth, MaxHealth);
	}

	return ActualDamage;
}
