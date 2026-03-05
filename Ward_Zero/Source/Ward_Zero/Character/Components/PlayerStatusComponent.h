#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatusComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDiedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDiedDelegate);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UPlayerStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerStatusComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region 스테미나 시스템 (Stamina)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status|Stamina")
	float MaxStamina = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status|Stamina")
	float CurrStamina;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status|Stamina")
	float StaminaDrainRate = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status|Stamina")
	float StaminaRegenRate = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status|Stamina")
	float MinStaminaToSprint = 20.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status|Stamina")
	bool bIsExhausted = false;

	bool CanSprint() const { return !bIsExhausted && CurrStamina > 0.0f; }
#pragma endregion

#pragma region 체력 시스템 (Health)
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float CurrHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	bool bIsDead = false;

	// 이벤트 
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDiedDelegate OnPlayerDied;

	float ApplyDamage(float DamageAmount);
	bool IsDead() const { return bIsDead; }
#pragma endregion

#pragma region Interaction - WJ
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedDelegate OnHealthChanged;
#pragma endregion 
};
