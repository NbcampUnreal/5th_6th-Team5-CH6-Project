#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCombatComponent.generated.h"

// 전방 선언 (Forward Declarations)
class AWeapon;
class UCameraComponent;
class UAnimMontage;
class UCameraShakeBase; // Fire 함수 파라미터에 사용되므로 추가

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

	// 재장전
	void Reload();
#pragma endregion

#pragma region 상태 확인 (Getters & State Checks)
	// 무기 장착 여부
	bool IsPistolEquipped() const { return bIsPistolEquipped; }

	// 조준 중인지 여부
	bool IsAiming() const { return bIsAiming; }

	// 현재 장착된 무기 가져오기
	AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	// 애니메이션용 Getter
	FVector GetHandIKTarget() const { return HandIKTargetLocation; }
	float GetAimYaw() const { return AimYaw; }
	float GetAimPitch() const { return AimPitch; }
#pragma endregion

#pragma region 무기 설정 및 상태 (Weapon Config & State)
	// 에디터에서 설정할 기본 무기 클래스 (예: BP_Pistol)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Config")
	TSubclassOf<AWeapon> DefaultWeaponClass;

	// 실제로 스폰되어 손에 들고 있는 무기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	bool bIsPistolEquipped = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	bool bIsAiming = false;
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
#pragma endregion

#pragma region 애니메이션 및 반동 설정 (Animation & Recoil Config)
	// 몽타주
	UPROPERTY(EditAnywhere, Category = "Combat|Animation")
	UAnimMontage* ReloadMontage;

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
};