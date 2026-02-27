#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

// 로코모션 방향 
UENUM(BlueprintType)
enum class ELocomotionDirection : uint8
{
	Forward, Backward, Left, Right
};

// 이동 상태 
UENUM(BlueprintType)
enum class ELocomotionState : uint8
{
	Walking, Running
};

// 방향별 Animation Set 
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
	
	// 상태 변경 함수 
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	void UpdateLocomotionState(ELocomotionState StateName);

	// Distance Matching 관련: 정지 애니메이션 시작 판단
	UFUNCTION(BlueprintPure, Category = "Movement|Distance Matching", meta = (BlueprintThreadSafe))
	bool ShouldDistanceMatchStop() const;

	UFUNCTION(BlueprintCallable, Category = "AnimNotify")
	void AnimNotify_StopQuickTurn();

protected:
	// 참조 변수 
	UPROPERTY(BlueprintReadOnly, Category = "Ref")
	TObjectPtr<class ACharacter> Character; //APrototypeCharacter 참조 -> ACharacter 참조 

	UPROPERTY(BlueprintReadOnly, Category = "Ref")
	TObjectPtr<class UCharacterMovementComponent> MovementComp;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	class AWeapon* EquippedWeapon;
	
	// 캐릭터 상태 변수 
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

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsClimbing;

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
	float BS_Direction; // 카메라 기준 방향

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float LocomotionAngle; // 액터 기준 방향

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ELocomotionDirection CurrentDir = ELocomotionDirection::Forward;

	// Warp & Distance Matching 관련
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	float DisplacementSinceLastUpdate;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float StartDistance;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float OrientationWarpingAngle;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float OrientationWarpingAlpha;

	// Pivot 관련 
	UPROPERTY(BlueprintReadOnly, Category = "Movement|Pivot")
	bool bIsPivoting;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Pivot")
	FVector LocalVelocity2D;

	// Distance Matching 노드용 캐시 데이터
	UPROPERTY(BlueprintReadOnly, Category = "Movement|Cached")
	FVector CachedLastUpdateVelocity;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Cached")
	bool bCachedUseSeparateBrakingFriction;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Cached")
	float CachedBrakingFriction;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Cached")
	float CachedGroundFriction;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Cached")
	float CachedBrakingFrictionFactor;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Cached")
	float CachedBrakingDecelerationWalking;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Weapon | Mesh")
	USkeletalMeshComponent* WeaponMesh;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UMirrorDataTable> MDT_FlashLight;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsMirroring = false; 

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float FlashlightAlpha = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float FlashlightIKAlpha = 0.0f;

public:
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	float SMGHandIKAlpha;
	
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FVector SMGHandIKLocation;

	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FRotator SMGHandIKRotation;
private:
	// Thread Safe
	void UpdateMovementCalculations(float DeltaSeconds);
	void UpdatePivotLogic();
	void UpdateOrientationWarping(float DeltaSeconds);
	void UpdateMovementDirection();
};