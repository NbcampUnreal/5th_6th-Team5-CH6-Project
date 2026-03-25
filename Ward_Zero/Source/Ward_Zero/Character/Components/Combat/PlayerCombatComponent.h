#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCombatComponent.generated.h"

class AWeapon;
class UCameraComponent;
class UAnimMontage;
class UCameraShakeBase;
class UAnimInstance;
class UCharacterCombatData;
class AProjectile;

USTRUCT(BlueprintType)
struct FSMGReloadAnimSet
{
	GENERATED_BODY()

public:
	// 제가 제안한 생성자
	FSMGReloadAnimSet()
		: StandingReload(nullptr)
		, StandingAimReload(nullptr)
		, CrouchReload(nullptr)
		, CrouchAimReload(nullptr)
	{
	}

	UPROPERTY(EditAnywhere, Category = "Reload") UAnimMontage* StandingReload;
	UPROPERTY(EditAnywhere, Category = "Reload") UAnimMontage* StandingAimReload;
	UPROPERTY(EditAnywhere, Category = "Reload") UAnimMontage* CrouchReload;
	UPROPERTY(EditAnywhere, Category = "Reload") UAnimMontage* CrouchAimReload;
};

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
	// 전투 로직
	void SetupCombat(UCameraComponent* InCamera);
	void SpawnDefaultWeapon();

	void ToggleEquip(UAnimMontage* Montage, UAnimInstance* AnimInst);
	void ChangeWeapon(int32 NewWeaponIndex, UAnimInstance* AnimInst);

	bool StartAiming();
	void StopAiming();

	// 발사 로직 (단발 및 연사 공용)
	void Fire(UAnimMontage* FireMontage, UAnimInstance* AnimInst, TSubclassOf<UCameraShakeBase> CamShake);
	void StartFire(UAnimMontage* InFireMontage, UAnimInstance* InAnimInst, TSubclassOf<UCameraShakeBase> InCamShake);
	void StopFire();

	void Reload();
	void CancelReload(UAnimInstance* AnimInst); 

	void HandleWeaponAttachment(bool bToHand);
	UAnimMontage* GetCurrentEquipMontage(bool bEquip);
	UAnimMontage* GetSelectedReloadMontage();

	// --- Getters ---
	bool IsPistolEquipped() const { return bIsWeaponDrawn && CurrentWeaponIndex == 1; }
	bool IsAiming() const { return bIsAiming; }
	bool IsFiring() const { return bIsFiring; }
	bool IsWeaponDrawn() const { return bIsWeaponDrawn; }
	bool IsRecoiling() const
	{
		return !FMath::IsNearlyEqual(CurrentRecoilRot.Pitch, TargetRecoilRot.Pitch, 0.01f) ||
			!FMath::IsNearlyEqual(CurrentRecoilRot.Yaw, TargetRecoilRot.Yaw, 0.01f);
	}

	void SetIsWeaponDrawn(bool bDrawn) { bIsWeaponDrawn = bDrawn; }
	AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
	FVector GetHandIKTarget() const { return HandIKTargetLocation; }
	int32 GetCurrentWeaponIndex() const { return CurrentWeaponIndex; }
	float GetAimYaw() const { return AimYaw; }
	float GetAimPitch() const { return AimPitch; }
	bool GetIsReloading() const;


private:
	void AutoFireLogic();
	void CalculateAimOffset();
	void UpdateHandIK();
	void HandleRecoil(float DeltaTime);
	void UpdateSpread(float DeltaTime);

protected:
	// 풀링 설정
	UPROPERTY(EditAnywhere, Category = "Combat|Pool")
	int32 ProjectilePoolSize = 30;

	UPROPERTY(EditAnywhere, Category = "Combat|Pool")
	TSubclassOf<AProjectile> ProjectileClass;

private:
	// 생성된 총알들을 보관하는 배열
	UPROPERTY()
	TArray<AProjectile*> ProjectilePool;

	// 풀 초기화
	void InitializeProjectilePool();

	// 풀에서 가용한 총알 가져오기
	AProjectile* GetProjectileFromPool();

public:
#pragma region 참조 클래스  
	UPROPERTY(EditAnywhere, Category = "Combat|Config") TSubclassOf<AWeapon> PistolClass;
	UPROPERTY(EditAnywhere, Category = "Combat|Config") TSubclassOf<AWeapon> SMGClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State") AWeapon* PistolWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State") AWeapon* SMGWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State") AWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere, Category = "Combat|Data") TObjectPtr<UCharacterCombatData> CombatData;
#pragma endregion 

#pragma region 몽타주 
	// Pistol 사격 몽타주
	UPROPERTY(EditAnywhere, Category = "Combat|Animation|Pistol") TObjectPtr<UAnimMontage> Pistol_FireMontage;
	UPROPERTY(EditAnywhere, Category = "Combat|Animation|Pistol") TObjectPtr<UAnimMontage> Pistol_CrouchFireMontage;
	// SMG 사격 몽타주
	UPROPERTY(EditAnywhere, Category = "Combat|Animation|SMG") TObjectPtr<UAnimMontage> SMG_FireMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Animation|SMG") TObjectPtr<UAnimMontage> SMG_CrouchFireMontage;
	UPROPERTY(EditAnywhere, Category = "Combat|Animation") UAnimMontage* Pistol_EquipMontage;
	UPROPERTY(EditAnywhere, Category = "Combat|Animation") UAnimMontage* Pistol_UnEquipMontage;
	UPROPERTY(EditAnywhere, Category = "Combat|Animation") UAnimMontage* SMG_EquipMontage;
	UPROPERTY(EditAnywhere, Category = "Combat|Animation") UAnimMontage* SMG_UnEquipMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Animation") UAnimMontage* Pistol_ReloadMontage;
	UPROPERTY(EditAnywhere, Category = "Combat|Animation") FSMGReloadAnimSet SMG_ReloadSet;
#pragma endregion

#pragma region 조준점 
	UPROPERTY(BlueprintReadOnly, Category = "Combat|State") float CurrentSpread = 0.0f;
#pragma endregion 

public:
	void SetCameraPitchLimit(bool IsAiming);
	
private:
	UPROPERTY() UCameraComponent* PlayerCamera;


	int32 CurrentWeaponIndex = 1;
	bool bIsWeaponDrawn = false;
	bool bIsAiming = false;
	bool bIsFiring = false;
	int32 CurrentShotsFired = 0;

	FVector HandIKTargetLocation;
	float AimYaw;
	float AimPitch;

	FRotator TargetRecoilRot = FRotator::ZeroRotator;
	FRotator CurrentRecoilRot = FRotator::ZeroRotator;
	FRotator LastRecoilRot = FRotator::ZeroRotator;

	FTimerHandle FireTimer;

	// 총 쏜 이후 다음 발사까지의 시간 간격을 제어하는 타이머
	float LastFireTime = 0.0f;

	UPROPERTY() UAnimMontage* CachedFireMontage;
	UPROPERTY() UAnimInstance* CachedAnimInst;
	TSubclassOf<UCameraShakeBase> CachedCamShake;

	UAnimMontage* GetSelectedFireMontage() const;

	// DataAsset에서 받아올 수치들 
	float MaxSpread = 5.0f;
	float MinSpread = 0.0f;
	float SpreadShrinkRate = 3.0f;
	float SpreadExpandRate = 15.0f;
	float FireSpreadPenalty = 2.5f;

	float RecoilInterpSpeed = 25.0f;

private:
	// 리펙토링  
	void PlayFireEffects(UAnimMontage* FireMontage, UAnimInstance* AnimInst, TSubclassOf<UCameraShakeBase> CamShake);
	void CalculateShotRecoil();
	void ProcessHit(const FHitResult& Hit, const FVector& ShotDir);
	void SpawnTracer(const FVector& Start, const FVector& End);

private:
	UPROPERTY()
	class ACharacter* CachedOwnerCharacter;
};