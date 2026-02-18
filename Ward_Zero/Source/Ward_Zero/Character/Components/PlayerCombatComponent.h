#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCombatComponent.generated.h"

class AWeapon;
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
	// 초기화 (Character에서 호출)
	void SetupCombat(UCameraComponent* InCamera);

	// 액션
<<<<<<< Updated upstream
	void ToggleEquip(UAnimMontage* Montage, UAnimInstance* AnimInst);
	void StartAiming();
	void StopAiming();

	// [중요] 발사 함수: 애니메이션과 쉐이크는 여기서, 실제 발사는 무기에게 위임
	void Fire(UAnimMontage* FireMontage, UAnimInstance* AnimInst, TSubclassOf<UCameraShakeBase> CamShake);

	void Reload();

	// 상태 확인 (Getter)
	bool IsPistolEquipped() const { return bIsPistolEquipped; }
	bool IsAiming() const { return bIsAiming; }

	// 애니메이션용 Getter
	FVector GetHandIKTarget() const { return HandIKTargetLocation; }
	float GetAimYaw() const { return AimYaw; }
	float GetAimPitch() const { return AimPitch; }

	// 현재 장착된 무기 가져오기
	AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

protected:
	// 무기 스폰 및 장착 헬퍼 함수
	void SpawnDefaultWeapon();

	// 조준 오프셋(AimOffset) 계산
	void CalculateAimOffset();

	// 레이저 사이트 위치 업데이트 (무기에서 가져옴)
	void UpdateHandIK();

	void HandleRecoil(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* ReloadMontage;

public:
	// 에디터에서 설정할 기본 무기 클래스 (예: BP_Pistol)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<AWeapon> DefaultWeaponClass;

	// 실제로 스폰되어 손에 들고 있는 무기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsPistolEquipped = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsAiming = false;

	// IK 및 애니메이션 관련 변수
	UPROPERTY(BlueprintReadOnly, Category = "Combat|IK")
	FVector HandIKTargetLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Aim")
	float AimYaw;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Aim")
	float AimPitch;

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
	float RecoilRecoverySpeed = 1.0f;

	FRotator TargetRecoilRot = FRotator::ZeroRotator;
	FRotator CurrentRecoilRot = FRotator::ZeroRotator;
	FRotator LastRecoilRot = FRotator::ZeroRotator;

private:
	UPROPERTY()
	UCameraComponent* PlayerCamera;
};