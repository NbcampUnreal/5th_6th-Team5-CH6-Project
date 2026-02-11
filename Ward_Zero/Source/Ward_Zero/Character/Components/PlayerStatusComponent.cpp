#include "Character/Components/PlayerStatusComponent.h"

UPlayerStatusComponent::UPlayerStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerStatusComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrHealth = MaxHealth;
	bIsDead = false;
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
	}

	return ActualDamage;
}
