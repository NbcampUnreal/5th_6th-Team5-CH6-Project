#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatusComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, Current, float, Max);
// 탄약 변경 델리게이트 (현재 탄수, 최대 탄창 용량, 예비 탄수)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAmmoChanged, int32, Current, int32, Max, int32, Reserve);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UPlayerStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerStatusComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	float ApplyDamage(float Amount);
	bool CanSprint() const { return !bIsExhausted && CurrStamina > 0.f; }
	bool IsDead() const { return bIsDead; }

	UPROPERTY(BlueprintAssignable) FOnPlayerDied OnPlayerDied;
	UPROPERTY(BlueprintAssignable) FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events|Ammo")
	FOnAmmoChanged OnPistolAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events|Ammo")
	FOnAmmoChanged OnSMGAmmoChanged;

	void UpdateAmmoUI(int32 WeaponIndex, int32 Current, int32 Max, int32 Reserve);

	UPROPERTY(EditAnywhere) float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere) float MaxStamina = 100.f;
	UPROPERTY(EditAnywhere) float StaminaDrainRate = 10.f;
	UPROPERTY(EditAnywhere) float StaminaRegenRate = 15.f;

	float CurrHealth;
	float CurrStamina;
	bool bIsDead = false;
	bool bIsExhausted = false;

public:
	UPROPERTY(EditAnywhere) float MinStaminaToSprint = 20.f;

	UFUNCTION(BlueprintCallable)
	void ReviveStatus(float HealthRatio = 1.0f);

	UPROPERTY(EditAnywhere, Category = "Montage | Revival")
	TObjectPtr<UAnimMontage> ReviveMontage;

public:
	// 부상 판정 기준 (30%)
	UPROPERTY(EditAnywhere, Category = "Status")
	float InjuryThreshold = 0.3f;

	bool IsInjured() const { return (CurrHealth / MaxHealth) <= InjuryThreshold; }
};