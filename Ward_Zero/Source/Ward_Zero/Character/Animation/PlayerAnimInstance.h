#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

UENUM(BlueprintType)
enum class ELocomotionDirection : uint8 { Forward, Backward, Left, Right };
UENUM(BlueprintType)
enum class ELocomotionState : uint8 { Walking, Running };
UENUM(BlueprintType)
enum class ETurnDirection : uint8 { Left, Right };

USTRUCT(BlueprintType)
struct FDirectionalAnimSet {
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Directional") TObjectPtr<UAnimSequence> Forward = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Directional") TObjectPtr<UAnimSequence> Backward = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Directional") TObjectPtr<UAnimSequence> Left = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Directional") TObjectPtr<UAnimSequence> Right = nullptr;
};

UCLASS()
class WARD_ZERO_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	UFUNCTION(BlueprintCallable, Category = "Locomotion") void UpdateLocomotionState(ELocomotionState StateName);

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Mesh")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float FlashlightAlpha = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float FlashlightAimIKAlpha = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float FlashlightRelaxIKAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FTransform PistolHandIKTransform;

	UPROPERTY(BlueprintReadOnly, Category = "IK")
	float PistolAlpha;
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	float SMGHandIKAlpha;
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FTransform SMGHandIKTransform;
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FVector PistolFlashlightIKTargetLoc;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float FallingTime;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void RequestLayerInertialBlend(float BlendTime = 0.2f);

	UFUNCTION() void AnimNotify_HealEffect();
	UFUNCTION() void AnimNotify_AttachItem();
	UFUNCTION() void AnimNotify_ConsumeItem();

	UPROPERTY(BlueprintReadOnly, Category = "PickUp")
	FVector PickupTargetLocation;
	UPROPERTY(BlueprintReadOnly, Category = "Interaction | IK")
	FVector CurrentPickUpLoc;
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	float PickupIKAlpha = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	float LeverIKAlpha = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FVector LeverTargetLocation;
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FRotator LeverTargetRotation;
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FVector DynamicLeverJointTarget;
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FVector DynamicPickupJointTarget;
	UPROPERTY(EditAnywhere, Category = "Animation|IK")
	float JointInterpSpeed = 10.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	float ButtonIKAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FVector ButtonIKTargetLocation;

	UFUNCTION() void AnimNotify_HideWeaponForLever();
	UFUNCTION() void AnimNotify_RestoreWeaponAfterLever();
	UFUNCTION() void AnimNotify_TriggerInteraction();
	UFUNCTION() void AnimNotify_EndInteraction();
	UFUNCTION() void AnimNotify_FreeMovement();

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Turn")
	bool bIsTurn;
	UPROPERTY(BlueprintReadOnly, Category = "Movement|Turn")
	float RootYawOffset;
	UPROPERTY(BlueprintReadOnly, Category = "Movement|Turn")
	ETurnDirection LocalTurnDir;

	UFUNCTION(BlueprintCallable, Category = "Movement|Turn") void CalculateYawDir();
	UFUNCTION(BlueprintCallable, Category = "Movement|Turn") void HandleTurnning();
	UFUNCTION(BlueprintCallable, Category = "Movement|Turn") void StopTurnIfMove();
	UFUNCTION(BlueprintCallable, Category = "Movement|Turn") void AnimNotify_TurnFinished();

protected:
	// References 
	UPROPERTY(BlueprintReadOnly, Category = "Ref") TObjectPtr<class ACharacter> Character;
	UPROPERTY(BlueprintReadOnly, Category = "Ref") TObjectPtr<class UCharacterMovementComponent> MovementComp;
	UPROPERTY(BlueprintReadOnly, Category = "Combat") TObjectPtr<class UPlayerCombatComponent> CombatComp;
	UPROPERTY(BlueprintReadOnly, Category = "Combat") class AWeapon* EquippedWeapon;
	UPROPERTY(BlueprintReadOnly, Category = "Ref") TObjectPtr<class APrototypeCharacter> CachedCharacter;

	// States & Data 
	UPROPERTY(BlueprintReadOnly, Category = "Movement") bool bIsCrouching;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") bool bIsGround;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") bool bIsRunning;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") bool bIsPistolEquipped;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") bool bIsQuickTurning;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") int32 TurnIndex;
	UPROPERTY(BlueprintReadOnly, Category = "Combat") bool bIsEquipping;
	UPROPERTY(BlueprintReadOnly, Category = "Combat") FVector HandIKTargetLocation;
	UPROPERTY(BlueprintReadOnly, Category = "Combat") bool bIsAiming;
	UPROPERTY(BlueprintReadOnly, Category = "Combat") bool bIsReloading;
	UPROPERTY(BlueprintReadOnly, Category = "FlashLight") bool bIsUseFlashLight;
	UPROPERTY(BlueprintReadOnly, Category = "Combat") bool bIsSMGEquipped;
	UPROPERTY(BlueprintReadOnly, Category = "Animation") bool bIsInteracting;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") bool bIsInjured;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") bool bIsWeaponDrawn;

	UPROPERTY(BlueprintReadOnly, Category = "Movement") bool bIsUseHeal;;
	UPROPERTY(BlueprintReadOnly, Category = "Combat") float AimPitch;
	UPROPERTY(BlueprintReadOnly, Category = "Combat") float AimYaw;
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK") float PistolIKAlpha = 0.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK") FVector PistolJointTarget;
	UPROPERTY(BlueprintReadOnly, Category = "Combat") bool bIsFiring;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") FVector Velocity;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") FVector Acceleration;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") float GroundSpeed;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") bool bIsAcceleration;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") bool bHasVelocity;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") bool bIsInVent;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") float BS_Direction;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") float LocomotionAngle;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") ELocomotionDirection CurrentDir = ELocomotionDirection::Forward;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") float OrientationWarpingAngle;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") float OrientationWarpingAlpha;
	UPROPERTY(BlueprintReadOnly, Category = "Movement") FVector LocalVelocity2D;
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Spread") float CurrSpread;

private:
	void UpdateMovementCalculations(float DeltaSeconds);
	void UpdateOrientationWarping(float DeltaSeconds);
	void UpdateMovementDirection();
	void UpdateEssentialData(float DeltaSeconds);
	void UpdateCombatIK(float DeltaSeconds);
	void UpdateInteractionIK(float DeltaSeconds);
	void UpdateFlashlightLogic(float DeltaSeconds);

	float TurnCooldownTimer = 0.0f;
	static constexpr float TurnCooldownDuration = 0.2f;
	bool bWasWeaponDrawnBeforeLever = false;
};