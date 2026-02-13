#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCombatComponent.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UCameraComponent;
class UAnimMontage;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UPlayerCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerCombatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 초기화
	void SetupCombat(UStaticMeshComponent* InPistolMesh, UCameraComponent* InCamera);

	// 액션
	void ToggleEquip(UAnimMontage* EquipMontage, UAnimInstance* AnimInst);
	void StartAiming();
	void StopAiming();
	void Fire(UAnimMontage* FireMontage, UAnimInstance* AnimInst, TSubclassOf<UCameraShakeBase> CamShake);

	// 상태 확인 (Getter)
	bool IsPistolEquipped() const { return bIsPistolEquipped; }
	bool IsAiming() const { return bIsAiming; }

	// 애니메이션용 Getter
	FVector GetHandIKTarget() const { return HandIKTargetLocation; }
	float GetAimYaw() const { return AimYaw; }
	float GetAimPitch() const { return AimPitch; }

protected:
	void UpdateLaserSight();
	void CalculateAimOffset();

	

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsPistolEquipped = false;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bIsAiming = false;

	UPROPERTY(EditAnywhere, Category = "Weapon|Effects")
	UNiagaraSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Weapon|Effects")
	UNiagaraSystem* ImpactEffect;

	// [중요] 레이저 사이트 에셋 (에디터에서 할당)
	UPROPERTY(EditAnywhere, Category = "Weapon|Effects")
	UNiagaraSystem* LaserSightSystem;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|IK")
	FVector HandIKTargetLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Aim")
	float AimYaw;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Aim")
	float AimPitch;

	UPROPERTY(EditAnywhere, Category="DamageType")
	TSubclassOf<UDamageType> DamageType;

private:
	UPROPERTY()
	UStaticMeshComponent* PistolMesh;

	UPROPERTY()
	UCameraComponent* PlayerCamera;

	UPROPERTY()
	UNiagaraComponent* LaserSightComponent;
};