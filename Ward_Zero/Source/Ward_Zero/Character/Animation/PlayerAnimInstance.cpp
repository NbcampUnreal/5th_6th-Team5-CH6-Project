#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h" // 기본 물리 데이터는 ACharacter에서 가져오기 
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Animation/Interface/PlayerAnimInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Weapon/Weapon.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<ACharacter>(TryGetPawnOwner());
	if (Character)
	{
		MovementComp = Character->GetCharacterMovement();
	}
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!Character || !MovementComp) return;

	// 인터페이스를 거쳐 데이터를 가져오기 
	if (IPlayerAnimInterface* AnimInterface = Cast<IPlayerAnimInterface>(TryGetPawnOwner()))
	{
		bIsRunning = AnimInterface->GetIsRunning();
		bIsCrouching = AnimInterface->GetIsCrouching();
		bIsGround = AnimInterface->GetIsGround();
		bIsPistolEquipped = AnimInterface->GetIsPistolEquipped();
		bIsQuickTurning = AnimInterface->GetIsQuickTurning();
		TurnIndex = AnimInterface->GetTurnIndex();
		bIsEquipping = AnimInterface->IsEquipping();
		HandIKTargetLocation = AnimInterface->GetHandIKTargetLoc();
		bIsAiming = AnimInterface->GetIsAiming();
		bIsClimbing = AnimInterface->GetIsClimbing();
		bIsReloading = AnimInterface->GetIsReloading();
		bIsUseFlashLight = AnimInterface->GetIsUseFlashLight();

		USkeletalMeshComponent* TempMesh = AnimInterface->GetEquippedWeaponMesh();
		WeaponMesh = IsValid(TempMesh) ? TempMesh : nullptr; 

		AWeapon* TempWeapon = AnimInterface->GetEquippedWeapon();
		EquippedWeapon = IsValid(TempWeapon) ? TempWeapon : nullptr;

		bIsSMGEquipped = AnimInterface->GetIsSMGEquipped();
	}

	float CurveValue = GetCurveValue(TEXT("HandIKLeftAlpha"));
	bool bCanUseSMGIK = bIsSMGEquipped && !bIsQuickTurning;

	if (bCanUseSMGIK)
	{
		float TargetAlpha = CurveValue;

		SMGHandIKAlpha = FMath::FInterpTo(SMGHandIKAlpha, TargetAlpha, DeltaSeconds, 15.0f);
	}
	else
	{
		SMGHandIKAlpha = FMath::FInterpTo(SMGHandIKAlpha, 0.0f, DeltaSeconds, 20.0f);
	}

	if (bIsPistolEquipped)
	{
		FlashlightIKAlpha = FMath::FInterpTo(FlashlightIKAlpha, CurveValue, DeltaSeconds, 15.0f);
	}

	UpdateMovementCalculations(DeltaSeconds);

	// 엔진 물리 데이터 동기화 
	Velocity = Character->GetVelocity();
	Acceleration = MovementComp->GetCurrentAcceleration();

	// Distance Matching 노드를 위한 무브먼트 데이터 캐싱
	CachedLastUpdateVelocity = MovementComp->GetLastUpdateVelocity();
	bCachedUseSeparateBrakingFriction = MovementComp->bUseSeparateBrakingFriction;
	CachedBrakingFriction = MovementComp->BrakingFriction;
	CachedGroundFriction = MovementComp->GroundFriction;
	CachedBrakingFrictionFactor = MovementComp->BrakingFrictionFactor;
	CachedBrakingDecelerationWalking = MovementComp->BrakingDecelerationWalking;

	//Unarmed FlashLight 
	float TargetAlpha = bIsUseFlashLight ? 1.0f : 0.0f;
	FlashlightAlpha = FMath::FInterpTo(FlashlightAlpha, TargetAlpha, DeltaSeconds, 5.0f);

	//Pistol FlashLight 
	float TargetIKAlpha = (bIsUseFlashLight && bIsPistolEquipped && !bIsEquipping && !bIsReloading) ? 1.0f : 0.0f;
	FlashlightIKAlpha = FMath::FInterpTo(FlashlightIKAlpha, TargetIKAlpha, DeltaSeconds, 10.0f);

	//SMG IK Alpha 
	bool bIKCondition = bIsSMGEquipped && !bIsReloading && !bIsEquipping && !bIsQuickTurning;
	float TargetSMGAlpha = bIKCondition ? 1.0f : 0.0f;

	if (bIsEquipping || !bIsSMGEquipped)
	{
		SMGHandIKAlpha = 0.0f;
	}
	else
	{
		float InterpSpeed = bIKCondition ? 5.0f : 20.0f; 
		SMGHandIKAlpha = FMath::FInterpTo(SMGHandIKAlpha, TargetSMGAlpha, DeltaSeconds, InterpSpeed);
	}
}

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	if (!Character) return;

	UpdateMovementCalculations(DeltaSeconds);
	UpdatePivotLogic();
	UpdateMovementDirection();
	UpdateOrientationWarping(DeltaSeconds);
}

void UPlayerAnimInstance::UpdateMovementCalculations(float DeltaSeconds)
{
	GroundSpeed = Velocity.Size2D();
	bHasVelocity = GroundSpeed > 5.0f;

	// 가속도 계산 (Z축 제외)
	const FVector Accel2D = Acceleration * FVector(1.f, 1.f, 0.f);
	bIsAcceleration = !Accel2D.IsNearlyZero();

	// Displacement 및 Start Distance
	DisplacementSinceLastUpdate = GroundSpeed * DeltaSeconds;
	if (bIsAcceleration) StartDistance += DisplacementSinceLastUpdate;
	else StartDistance = 0.0f;

	// 로컬 속도 (Pivot 및 Warping용)
	LocalVelocity2D = Character->GetActorRotation().UnrotateVector(Velocity);

	// 입력 방향 벡터 결정
	const FVector DirectionVector = (GroundSpeed < 10.0f && bIsAcceleration) ? Acceleration : Velocity;

	// 방향 각도 계산
	const FRotator ControlRotationYaw = FRotator(0.f, Character->GetControlRotation().Yaw, 0.f);
	BS_Direction = UKismetAnimationLibrary::CalculateDirection(DirectionVector, ControlRotationYaw);
	LocomotionAngle = UKismetAnimationLibrary::CalculateDirection(DirectionVector, Character->GetActorRotation());
}

void UPlayerAnimInstance::UpdatePivotLogic()
{
	if (bIsAcceleration && bHasVelocity)
	{
		const FVector VelocityDir = Velocity.GetSafeNormal2D();
		const FVector AccelDir = Acceleration.GetSafeNormal2D();
		const float Dot = FVector::DotProduct(VelocityDir, AccelDir);

		// Hysteresis 적용 피벗 감지
		bIsPivoting = bIsPivoting ? (Dot < 0.0f) : (Dot < -0.5f);
	}
	else
	{
		bIsPivoting = false;
	}
}

void UPlayerAnimInstance::UpdateMovementDirection()
{
	if (GroundSpeed < 1.0f)
	{
		if (FMath::IsWithinInclusive(BS_Direction, -45.f, 45.f)) CurrentDir = ELocomotionDirection::Forward;
		else if (FMath::IsWithinInclusive(BS_Direction, 45.f, 135.f)) CurrentDir = ELocomotionDirection::Right;
		else if (FMath::IsWithinInclusive(BS_Direction, -135.f, -45.f)) CurrentDir = ELocomotionDirection::Left;
		else CurrentDir = ELocomotionDirection::Backward;
		return;
	}

	switch (CurrentDir)
	{
	case ELocomotionDirection::Forward:
		if (BS_Direction > 70.0f) CurrentDir = ELocomotionDirection::Right;
		else if (BS_Direction < -70.0f) CurrentDir = ELocomotionDirection::Left;
		break;
	case ELocomotionDirection::Right:
		if (BS_Direction < 20.0f) CurrentDir = ELocomotionDirection::Forward;
		else if (BS_Direction > 160.0f) CurrentDir = ELocomotionDirection::Backward;
		break;
	case ELocomotionDirection::Left:
		if (BS_Direction > -20.0f) CurrentDir = ELocomotionDirection::Forward;
		else if (BS_Direction < -160.0f) CurrentDir = ELocomotionDirection::Backward;
		break;
	case ELocomotionDirection::Backward:
		if (FMath::IsWithin(BS_Direction, 0.f, 110.f)) CurrentDir = ELocomotionDirection::Right;
		else if (FMath::IsWithin(BS_Direction, -110.f, 0.f)) CurrentDir = ELocomotionDirection::Left;
		break;
	}
}

void UPlayerAnimInstance::UpdateOrientationWarping(float DeltaSeconds)
{
	float TargetAngle = 0.0f;
	switch (CurrentDir)
	{
	case ELocomotionDirection::Forward:  TargetAngle = 0.0f; break;
	case ELocomotionDirection::Right:    TargetAngle = 90.0f; break;
	case ELocomotionDirection::Left:     TargetAngle = -90.0f; break;
	case ELocomotionDirection::Backward: TargetAngle = (LocomotionAngle > 0) ? 180.0f : -180.0f; break;
	}

	OrientationWarpingAngle = FRotator::NormalizeAxis(LocomotionAngle - TargetAngle);

	const float TargetAlpha = bHasVelocity ? 1.0f : 0.0f;
	OrientationWarpingAlpha = FMath::FInterpTo(OrientationWarpingAlpha, TargetAlpha, DeltaSeconds, 10.0f);
}

bool UPlayerAnimInstance::ShouldDistanceMatchStop() const
{
	return bHasVelocity && !bIsAcceleration;
}

void UPlayerAnimInstance::AnimNotify_StopQuickTurn()
{
	if (IPlayerAnimInterface* AnimInterface = Cast<IPlayerAnimInterface>(TryGetPawnOwner()))
	{
		AnimInterface->SetIsQuickTurning(false);

		bIsQuickTurning = false;
	}
}

void UPlayerAnimInstance::UpdateLocomotionState(ELocomotionState StateName)
{
	bIsRunning = (StateName == ELocomotionState::Running);
}

void UPlayerAnimInstance::UpdateSMGHandIK(float DeltaSeconds)
{
	if (WeaponMesh && Character && Character->GetMesh())
	{
		// Transform Space는 RTS_World 기준
		FTransform SocketTransform = WeaponMesh->GetSocketTransform(TEXT("SMG_LeftHand"), RTS_World);

		// World Space 위치를 hand_r 뼈대 공간으로 변환 
		FVector OutPosition;
		FRotator OutRotation;

		Character->GetMesh()->TransformToBoneSpace(
			TEXT("hand_r"),
			SocketTransform.GetLocation(),
			SocketTransform.Rotator(),
			OutPosition,
			OutRotation
		);

		SMGHandIKTransform = FTransform(OutRotation, OutPosition, FVector(1.0f, 1.0f, 1.0f));
	}
}