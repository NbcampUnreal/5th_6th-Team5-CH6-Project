#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "PlayerAnimInstance.generated.h"

UENUM(BlueprintType)
enum class ELocomotionDirection : uint8
{
	Forward, Backward, Left, Right
};

UENUM(BlueprintType)
enum class ELocomotionState : uint8
{
	Walking, Running
};

USTRUCT(BlueprintType)
struct FDirectionalAnimSet
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Directional")
	TObjectPtr<UAnimSequence> Forward = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Directional")
	TObjectPtr<UAnimSequence> Backward = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Directional")
	TObjectPtr<UAnimSequence> Left = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Directional")
	TObjectPtr<UAnimSequence> Right = nullptr;
};

UCLASS()
class WARD_ZERO_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	void UpdateLocomotionState(ELocomotionState StateName);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Ref")
	TObjectPtr<class ACharacter> Character;

	UPROPERTY(BlueprintReadOnly, Category = "Ref")
	TObjectPtr<class UCharacterMovementComponent> MovementComp;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TObjectPtr<class UPlayerCombatComponent> CombatComp;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsGround;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsPistolEquipped;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsQuickTurning;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	int32 TurnIndex;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsEquipping;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FVector HandIKTargetLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsReloading;

	UPROPERTY(BlueprintReadOnly, Category = "FlashLight")
	bool bIsUseFlashLight;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsSMGEquipped;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsInteracting;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float AimYaw;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	float PistolIKAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsFiring;

	// 물리 및 이동 데이터
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector Velocity;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector Acceleration;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsAcceleration;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bHasVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float BS_Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float LocomotionAngle;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ELocomotionDirection CurrentDir = ELocomotionDirection::Forward;

	// ★ 원본에 있던 Distance Matching용 변수 복구
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	float DisplacementSinceLastUpdate;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float StartDistance;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float OrientationWarpingAngle;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float OrientationWarpingAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector LocalVelocity2D;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Spread")
	float CurrSpread;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Mesh")
	USkeletalMeshComponent* WeaponMesh;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float FlashlightAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float FlashlightIKAlpha = 0.0f;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FTransform PistolHandIKTransform;

public:
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	float PistolAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "IK")
	float SMGHandIKAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FTransform SMGHandIKTransform;



public:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float FallingTime;

public:
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void RequestLayerInertialBlend(float BlendTime = 0.2f);
private:
	// Thread Safe 함수들 - 원본과 동일하게 유지
	void UpdateMovementCalculations(float DeltaSeconds);
	void UpdateOrientationWarping(float DeltaSeconds);
	void UpdateMovementDirection();
};