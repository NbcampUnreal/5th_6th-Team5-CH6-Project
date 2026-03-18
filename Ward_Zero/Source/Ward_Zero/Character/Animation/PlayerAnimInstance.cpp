#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Animation/Interface/PlayerAnimInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Weapon/Weapon.h"
#include "Animation/AnimNode_Inertialization.h" 

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<ACharacter>(TryGetPawnOwner());
	CachedCharacter = Cast<APrototypeCharacter>(Character);

	if (Character)
	{
		MovementComp = Character->GetCharacterMovement();
	}
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!Character || !MovementComp) return;

	// 기초 데이터 계산 
	Velocity = Character->GetVelocity();
	Acceleration = MovementComp->GetCurrentAcceleration();
	if (Velocity.SizeSquared() < 1.0f)
	{
		GroundSpeed = 0.0f;
		bHasVelocity = false;
	}
	else {
		UpdateMovementCalculations(DeltaSeconds);
	}

	// 인터페이스 상태 캐싱
	if (IPlayerAnimInterface* AnimInterface = Cast<IPlayerAnimInterface>(TryGetPawnOwner()))
	{
		bIsRunning = AnimInterface->GetIsRunning();
		bIsCrouching = AnimInterface->GetIsCrouching();
		bIsGround = AnimInterface->GetIsGround();
		bIsPistolEquipped = AnimInterface->GetIsPistolEquipped();
		bIsQuickTurning = AnimInterface->GetIsQuickTurning();
		TurnIndex = AnimInterface->GetTurnIndex();
		bIsEquipping = AnimInterface->IsEquipping();
		bIsAiming = AnimInterface->GetIsAiming();
		bIsReloading = AnimInterface->GetIsReloading();
		bIsUseFlashLight = AnimInterface->GetIsUseFlashLight();
		bIsFiring = AnimInterface->IsFiring();
		AimPitch = AnimInterface->GetAimPitch();
		AimYaw = AnimInterface->GetAimYaw();
		CurrSpread = AnimInterface->GetCurrSpread();
		CombatComp = AnimInterface->GetCombatComp();
		bIsWeaponDrawn = AnimInterface->GetbIsWeaponDrawn();
		bIsInjured = AnimInterface->GetIsInjured();

		WeaponMesh = AnimInterface->GetEquippedWeaponMesh();
		EquippedWeapon = AnimInterface->GetEquippedWeapon();
		bIsSMGEquipped = AnimInterface->GetIsSMGEquipped();

		AimPitch = bIsReloading ? FMath::FInterpTo(AimPitch, 0.0f, DeltaSeconds, 5.0f) : AnimInterface->GetAimPitch();
	}

	// IK를 끄는 Busy 조건 
	bool bIsIKBusy = bIsEquipping || bIsReloading || bIsInteracting || bIsQuickTurning;

	// SMG IK
	float CurveValue = GetCurveValue(TEXT("HandIKLeftAlpha"));
	bool bCanUseSMGIK = bIsSMGEquipped && !bIsQuickTurning;
	SMGHandIKAlpha = FMath::FInterpTo(SMGHandIKAlpha, (bCanUseSMGIK && !bIsIKBusy) ? CurveValue : 0.0f, DeltaSeconds, 15.0f);

	// Pistol IK 조건
	bool bPistolIKCondition = bIsPistolEquipped && !bIsSMGEquipped
		&& !bIsEquipping && !bIsReloading && !bIsInteracting
		&& (!bIsUseFlashLight || !bIsAiming || bIsRunning);

	PistolIKAlpha = FMath::FInterpTo(PistolIKAlpha, bPistolIKCondition ? 1.0f : 0.0f, DeltaSeconds, 15.0f);

	// Flashlight IK
	// 조준 중 손전등 IK 
	bool bFlashlightAimCondition = bIsUseFlashLight && bIsPistolEquipped && bIsAiming && !bIsSMGEquipped && !bIsIKBusy;
	FlashlightAimIKAlpha = FMath::FInterpTo(FlashlightAimIKAlpha, bFlashlightAimCondition ? 1.0f : 0.0f, DeltaSeconds, 20.0f);

	// Flashlight Alpha 
	float TargetFlashAlpha = (bIsUseFlashLight && !bIsSMGEquipped && (!bIsPistolEquipped || bIsAiming)) ? 1.0f : 0.0f;
	FlashlightAlpha = FMath::FInterpTo(FlashlightAlpha, TargetFlashAlpha, DeltaSeconds, 5.0f);

	FlashlightRelaxIKAlpha = FMath::FInterpTo(FlashlightRelaxIKAlpha, 0.0f, DeltaSeconds, 20.0f);

	// 소켓 및 관절 위치 업데이트 
	if (bIsPistolEquipped && WeaponMesh)
	{
		FName TargetSocketName = bIsAiming ? FName("FlashLightIKSocket") : FName("RelaxFlashLightIK_Socket");
		PistolFlashlightIKTargetLoc = WeaponMesh->GetSocketLocation(TargetSocketName);

		FVector TargetJointPos = bIsAiming ? FVector(0.f, -20.f, 0.f) : FVector(0.f, -280.f, -150.f);
		PistolJointTarget = FMath::VInterpTo(PistolJointTarget, TargetJointPos, DeltaSeconds, 15.0f);
	}

	// 아이템 픽업 IK 
	if (CachedCharacter)
	{
		PickupTargetLocation = CachedCharacter->CurrentPickupLocation;
		float PickupCurveValue = GetCurveValue(TEXT("PickupIK"));
		PickupIKAlpha = FMath::FInterpTo(PickupIKAlpha, PickupCurveValue, DeltaSeconds, 15.0f);

		if (PickupIKAlpha > 0.1f)
		{
			FVector NewJointTarget;
			FVector LocalTargetPos = CachedCharacter->GetActorTransform().InverseTransformPosition(PickupTargetLocation);
			if (LocalTargetPos.Z < -50.0f) NewJointTarget = FVector(-20.0f, -60.0f, 40.0f);
			else if (LocalTargetPos.Z < 30.0f) NewJointTarget = FVector(-50.0f, -50.0f, 0.0f);
			else NewJointTarget = FVector(-40.0f, -40.0f, -30.0f);
			DynamicPickupJointTarget = FMath::VInterpTo(DynamicPickupJointTarget, NewJointTarget, DeltaSeconds, 15.0f);
		}
	}
}
void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	if (!Character) return;

	UpdateMovementCalculations(DeltaSeconds);
	UpdateMovementDirection();
	UpdateOrientationWarping(DeltaSeconds);
}

void UPlayerAnimInstance::UpdateMovementCalculations(float DeltaSeconds)
{
	GroundSpeed = Velocity.Size2D();

	float VelocityThreshold = bIsCrouching ? 12.0f : 5.0f;
	bHasVelocity = GroundSpeed > VelocityThreshold;

	const FVector Accel2D = Acceleration * FVector(1.f, 1.f, 0.f);
	bIsAcceleration = Accel2D.Size() > 10.0f;
	LocalVelocity2D = Character->GetActorRotation().UnrotateVector(Velocity);

	const FVector DirectionVector = (GroundSpeed < 15.0f && bIsAcceleration) ? Acceleration : Velocity;
	const FRotator ControlRotationYaw = FRotator(0.f, Character->GetControlRotation().Yaw, 0.f);
	BS_Direction = UKismetAnimationLibrary::CalculateDirection(DirectionVector, ControlRotationYaw);
	LocomotionAngle = UKismetAnimationLibrary::CalculateDirection(DirectionVector, Character->GetActorRotation());

	if (!bIsGround)
		FallingTime += DeltaSeconds;
	else
		FallingTime = 0.0f;
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

	// 달리기가 아닐 때만 방향에 따른 각도를 계산합니다.
	if (!bIsRunning)
	{
		switch (CurrentDir)
		{
		case ELocomotionDirection::Forward:  TargetAngle = 0.0f; break;
		case ELocomotionDirection::Right:    TargetAngle = 90.0f; break;
		case ELocomotionDirection::Left:     TargetAngle = -90.0f; break;
		case ELocomotionDirection::Backward: TargetAngle = (LocomotionAngle > 0) ? 180.0f : -180.0f; break;
		}
	}
	// bIsRunning일 때는 TargetAngle이 0이 되므로, 
	// LocomotionAngle(캡슐과 이동방향의 차이)만큼만 최소한으로 보정하게 됩니다.

	OrientationWarpingAngle = FRotator::NormalizeAxis(LocomotionAngle - TargetAngle);

	// 달리기 중에는 Warping 기능을 완전히 끕니다 (Alpha = 0)
	const float TargetAlpha = (bHasVelocity && !bIsRunning) ? 1.0f : 0.0f;
	OrientationWarpingAlpha = FMath::FInterpTo(OrientationWarpingAlpha, TargetAlpha, DeltaSeconds, 10.0f);
}

void UPlayerAnimInstance::UpdateLocomotionState(ELocomotionState StateName)
{
	bIsRunning = (StateName == ELocomotionState::Running);
}

void UPlayerAnimInstance::RequestLayerInertialBlend(float BlendTime)
{
	FInertializationRequest Request;
	Request.Duration = BlendTime;

	UE::Anim::IInertializationRequester* Requester = (UE::Anim::IInertializationRequester*)this;

	if (Requester)
	{
		Requester->RequestInertialization(Request);
	}
}

void UPlayerAnimInstance::AnimNotify_HealEffect()
{
	if (IPlayerAnimInterface* AnimInterface = Cast<IPlayerAnimInterface>(TryGetPawnOwner()))
	{
		AnimInterface->ExecuteHealPoint();
	}
}

void UPlayerAnimInstance::AnimNotify_AttachItem()
{
	if (CachedCharacter)
	{
		CachedCharacter->AttachInteractingItem();
	}
}

void UPlayerAnimInstance::AnimNotify_ConsumeItem()
{
	if (CachedCharacter)
	{
		CachedCharacter->ConsumeInteractingItem();
	}
}