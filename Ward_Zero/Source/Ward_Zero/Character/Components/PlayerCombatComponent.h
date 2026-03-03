#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCombatComponent.generated.h"
//#include "Character/Components/InteractionComponent.h"

// 전방 선언 (Forward Declarations)
class AWeapon;
class UCameraComponent;
class UAnimMontage;
class UCameraShakeBase; // Fire 함수 파라미터에 사용되므로 추가

USTRUCT(BlueprintType)
struct FSMGReloadAnimSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Reload")
	UAnimMontage* StandingReload;      // 서서 일반

	UPROPERTY(EditAnywhere, Category = "Reload")
	UAnimMontage* StandingAimReload;   // 서서 조준 중

	UPROPERTY(EditAnywhere, Category = "Reload")
	UAnimMontage* CrouchReload;        // 앉아서 일반

	UPROPERTY(EditAnywhere, Category = "Reload")
	UAnimMontage* CrouchAimReload;     // 앉아서 조준 중
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WARD_ZERO_API UPlayerCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerCombatComponent();

protected:
#pragma region 기본 컴포넌트 함수 (Component Overrides)
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
#pragma endregion

public:
#pragma region 초기화 및 전투 액션 (Initialization & Combat Actions)
	// 초기화 (Character에서 호출)
	void SetupCombat(UCameraComponent* InCamera);

	// 무기 장착/해제 토글
	void ToggleEquip(UAnimMontage* Montage, UAnimInstance* AnimInst);

	// 조준 (우클릭)
	bool StartAiming();
	void StopAiming();

	// [중요] 발사 함수: 애니메이션과 쉐이크는 여기서, 실제 발사는 무기에게 위임
	void Fire(UAnimMontage* FireMontage, UAnimInstance* AnimInst, TSubclassOf<UCameraShakeBase> CamShake);

	void StartFire(UAnimMontage* InFireMontage, UAnimInstance* InAnimInst, TSubclassOf<UCameraShakeBase> InCamShake);
	void StopFire();

	// 재장전
	void Reload();

public:
	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	UAnimMontage* Pistol_FireMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	UAnimMontage* SMG_FireMontage;

private:
	FTimerHandle FireTimer;
	void AutoFireLogic();

	UPROPERTY()	
	UAnimMontage* CachedFireMontage;

	UPROPERTY()	
	UAnimInstance* CachedAnimInst;

	TSubclassOf<UCameraShakeBase> CachedCamShake;
#pragma endregion

public:
	UPROPERTY(EditAnywhere, Category = "Combat|Animation")
	UAnimMontage* Pistol_EquipMontage;
	UPROPERTY(EditAnywhere, Category = "Combat|Animation")
	UAnimMontage* Pistol_UnEquipMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Animation")
	UAnimMontage* SMG_EquipMontage;
	UPROPERTY(EditAnywhere, Category = "Combat|Animation")
	UAnimMontage* SMG_UnEquipMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Animation")
	UAnimMontage* Melee_EquipMontage;
	UPROPERTY(EditAnywhere, Category = "Combat|Animation")
	UAnimMontage* Melee_UnEquipMontage;

	UAnimMontage* GetCurrentEquipMontage(bool bEquip);

public:
#pragma region 재장전 애니메이션 
	UPROPERTY(EditAnywhere, Category = "Combat|Animation|Pistol")
	UAnimMontage* Pistol_ReloadMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Animation|SMG")
	FSMGReloadAnimSet SMG_ReloadSet;

	UAnimMontage* GetSelectedReloadMontage();

public:
	void HandleWeaponAttachment(bool bToHand);
#pragma endregion

public:
#pragma region 상태 확인 (Getters & State Checks)
	// 무기 장착 여부
	bool IsPistolEquipped() const { return bIsWeaponDrawn && CurrentWeaponIndex == 1; }

	// 조준 중인지 여부
	bool IsAiming() const { return bIsAiming; }
	bool IsFiring() const { return bIsFiring; }

	bool IsWeaponDrawn() const { return bIsWeaponDrawn; }
	void SetIsWeaponDrawn(bool bDrawn) { bIsWeaponDrawn = bDrawn; }

	// 현재 장착된 무기 가져오기
	AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	// 애니메이션용 Getter
	FVector GetHandIKTarget() const { return HandIKTargetLocation; }
	float GetAimYaw() const { return AimYaw; }
	float GetAimPitch() const { return AimPitch; }
	bool GetIsReloading() const;
	int32 GetCurrentWeaponIndex() const { return CurrentWeaponIndex; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	bool bIsFiring = false;
#pragma endregion

#pragma region 무기 설정 및 상태 (Weapon Config & State)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Config")
	TSubclassOf<AWeapon> PistolClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Config")
	TSubclassOf<AWeapon> SMGClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	AWeapon* PistolWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	AWeapon* SMGWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	int32 CurrentWeaponIndex = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	bool bIsWeaponDrawn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	bool bIsAiming = false;

	void ChangeWeapon(int32 NewWeaponIndex, UAnimInstance* AnimInst);
	
#pragma endregion

#pragma region IK 및 조준 오프셋 (IK & Aim Offset)
	UPROPERTY(BlueprintReadOnly, Category = "Combat|IK")
	FVector HandIKTargetLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Aim")
	float AimYaw;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Aim")
	float AimPitch;
#pragma endregion

protected:
#pragma region 내부 헬퍼 함수 (Internal Helpers)
	// 무기 스폰 및 장착 헬퍼 함수
	void SpawnDefaultWeapon();

	// 조준 오프셋(AimOffset) 계산
	void CalculateAimOffset();

	// 레이저 사이트 위치 업데이트 (무기에서 가져옴)
	void UpdateHandIK();

	// 반동 처리
	void HandleRecoil(float DeltaTime);

	void UpdateSpread(float DeltaTime);

	int32 CurrentShotsFired = 0;
#pragma endregion

#pragma region 애니메이션 및 반동 설정 (Animation & Recoil Config)
	// 반동 설정
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Recoil")
	float MinRecoilPitch = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Recoil")
	float MaxRecoilPitch = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Recoil")
	float MinRecoilYaw = -3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Recoil")
	float MaxRecoilYaw = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Recoil")
	float RecoilInterpSpeed = 25.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Recoil")
	float RecoilRecoverySpeed = 5.0f;
#pragma endregion

private:
#pragma region 내부 처리용 변수 (Internal Data)
	// 반동 계산용 내부 상태 변수들 (외부 접근 불필요)
	FRotator TargetRecoilRot = FRotator::ZeroRotator;
	FRotator CurrentRecoilRot = FRotator::ZeroRotator;
	FRotator LastRecoilRot = FRotator::ZeroRotator;

	UPROPERTY()
	UCameraComponent* PlayerCamera;
#pragma endregion

public:
#pragma region 조준점 스프레드 (Crosshair Spread)

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Spread")
		float CurrentSpread = 0.0f;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Spread")
	float MaxSpread = 5.0f; // 최대 벌어짐 각도

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Spread")
	float MinSpread = 0.0f; // 최소 벌어짐 각도 0 = 100% 정확

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Spread")
	float SpreadShrinkRate = 3.0f; // 가만히 있을 때 조준선이 모이는 속도

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Spread")
	float SpreadExpandRate = 15.0f; // 움직일 때 조준선이 벌어지는 속도

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Spread")
	float FireSpreadPenalty = 2.5f; // 총을 한 발 쏠 때마다 반동으로 벌어지는 패널티 수치
#pragma endregion

//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Interaction")
//UInteractionComponent* InteractionComp;
};