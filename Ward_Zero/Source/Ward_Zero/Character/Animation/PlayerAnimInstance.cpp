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
	if (Character)
	{
		MovementComp = Character->GetCharacterMovement();
	}
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!Character || !MovementComp) return;

	// 1. 기초 데이터 계산 (원본 유지)
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

	// 2. 인터페이스 상태 캐싱 (원본 유지)
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

	// [해결 1] IK를 끄는 Busy 조건 (사격 bIsFiring은 여기서 제외해야 사격 시 안 깜빡임)
	bool bIsIKBusy = bIsEquipping || bIsReloading || bIsInteracting || bIsQuickTurning;

	// 3. SMG IK (원본 유지)
	float CurveValue = GetCurveValue(TEXT("HandIKLeftAlpha"));
	bool bCanUseSMGIK = bIsSMGEquipped && !bIsQuickTurning;
	SMGHandIKAlpha = FMath::FInterpTo(SMGHandIKAlpha, (bCanUseSMGIK && !bIsIKBusy) ? CurveValue : 0.0f, DeltaSeconds, 15.0f);

	// 4. Pistol IK 조건
	// 비조준 시 손전등을 켜도 양손 파지를 유지하도록 (!bIsUseFlashLight || !bIsAiming) 조건 유지
	bool bPistolIKCondition = bIsPistolEquipped && !bIsSMGEquipped
		&& !bIsEquipping && !bIsReloading && !bIsInteracting
		&& (!bIsUseFlashLight || !bIsAiming || bIsRunning);

	PistolIKAlpha = FMath::FInterpTo(PistolIKAlpha, bPistolIKCondition ? 1.0f : 0.0f, DeltaSeconds, 15.0f);

	// 5. Flashlight IK 및 Additive 포즈 조건

	// [A] 조준 중 손전등 IK (Harries Grip) : 원본 조건 복구
	bool bFlashlightAimCondition = bIsUseFlashLight && bIsPistolEquipped && bIsAiming && !bIsSMGEquipped && !bIsIKBusy;
	FlashlightAimIKAlpha = FMath::FInterpTo(FlashlightAimIKAlpha, bFlashlightAimCondition ? 1.0f : 0.0f, DeltaSeconds, 20.0f);

	// [해결 2] Flashlight Alpha (Additive 포즈) : 
	// 권총 비조준 시에는 이 값이 1이 되면 손이 뒤틀림. (Pistol + Relax 상황에서 0이 되도록 수정)
	float TargetFlashAlpha = (bIsUseFlashLight && !bIsSMGEquipped && (!bIsPistolEquipped || bIsAiming)) ? 1.0f : 0.0f;
	FlashlightAlpha = FMath::FInterpTo(FlashlightAlpha, TargetFlashAlpha, DeltaSeconds, 5.0f);

	FlashlightRelaxIKAlpha = FMath::FInterpTo(FlashlightRelaxIKAlpha, 0.0f, DeltaSeconds, 20.0f);

	// 6. 소켓 및 관절 위치 업데이트 (원본 로직 유지)
	if (bIsPistolEquipped && WeaponMesh)
	{
		FName TargetSocketName = bIsAiming ? FName("FlashLightIKSocket") : FName("RelaxFlashLightIK_Socket");
		PistolFlashlightIKTargetLoc = WeaponMesh->GetSocketLocation(TargetSocketName);

		FVector TargetJointPos = bIsAiming ? FVector(0.f, -20.f, 0.f) : FVector(0.f, -280.f, -150.f);
		PistolJointTarget = FMath::VInterpTo(PistolJointTarget, TargetJointPos, DeltaSeconds, 15.0f);
	}

	// 7. 아이템 픽업 IK (원본 유지)
	if (APrototypeCharacter* Player = Cast<APrototypeCharacter>(TryGetPawnOwner()))
	{
		PickupTargetLocation = Player->CurrentPickupLocation;
		float PickupCurveValue = GetCurveValue(TEXT("PickupIK"));
		PickupIKAlpha = FMath::FInterpTo(PickupIKAlpha, PickupCurveValue, DeltaSeconds, 15.0f);

		if (PickupIKAlpha > 0.1f)
		{
			FVector NewJointTarget;
			FVector LocalTargetPos = Player->GetActorTransform().InverseTransformPosition(PickupTargetLocation);
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
	switch (CurrentDir)
	{
	case ELocomotionDirection::Forward:  TargetAngle = 0.0f; break;
	case ELocomotionDirection::Right:    TargetAngle = 90.0f; break;
	case ELocomotionDirection::Left:     TargetAngle = -90.0f; break;
	case ELocomotionDirection::Backward: TargetAngle = (LocomotionAngle > 0) ? 180.0f : -180.0f; break;
	}

	OrientationWarpingAngle = FRotator::NormalizeAxis(LocomotionAngle - TargetAngle);

	const float TargetAlpha = bHasVelocity ? 1.0f : 0.0f;
	OrientationWarpingAlpha = FMath::FInterpTo(OrientationWarpingAlpha, TargetAlpha, DeltaSeconds, 6.0f);
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
	if (APrototypeCharacter* Player = Cast<APrototypeCharacter>(TryGetPawnOwner()))
	{
		Player->AttachInteractingItem();
	}
}

void UPlayerAnimInstance::AnimNotify_ConsumeItem()
{
	if (APrototypeCharacter* Player = Cast<APrototypeCharacter>(TryGetPawnOwner()))
	{
		Player->ConsumeInteractingItem();
	}
}