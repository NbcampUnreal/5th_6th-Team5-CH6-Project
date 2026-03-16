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

	// 1. 기초 물리 값 계산
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

	// 2. 인터페이스를 통한 상태 캐싱
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

		USkeletalMeshComponent* TempMesh = AnimInterface->GetEquippedWeaponMesh();
		WeaponMesh = IsValid(TempMesh) ? TempMesh : nullptr;

		AWeapon* TempWeapon = AnimInterface->GetEquippedWeapon();
		EquippedWeapon = IsValid(TempWeapon) ? TempWeapon : nullptr;

		bIsSMGEquipped = AnimInterface->GetIsSMGEquipped();

		// 장전 중에는 AimPitch 보정
		AimPitch = bIsReloading ? FMath::FInterpTo(AimPitch, 0.0f, DeltaSeconds, 5.0f) : AnimInterface->GetAimPitch();
	}

	bool bIsBusy = bIsRunning || bIsEquipping || bIsReloading || bIsInteracting;
	bool bCanUseIK = !bIsQuickTurning && !bIsBusy;

	// SMG IK 
	float TargetSMGAlpha = (bIsSMGEquipped && bCanUseIK) ? GetCurveValue(TEXT("HandIKLeftAlpha")) : 0.0f;
	SMGHandIKAlpha = FMath::FInterpTo(SMGHandIKAlpha, TargetSMGAlpha, DeltaSeconds, 15.0f);

	// Pistol & Flashlight IK 
	// Pistol Aiming FlashLight IK 
	bool bFlashlightAimCondition = bIsUseFlashLight && bIsPistolEquipped && bIsAiming && !bIsSMGEquipped && !bIsBusy;
	FlashlightAimIKAlpha = FMath::FInterpTo(FlashlightAimIKAlpha, bFlashlightAimCondition ? 1.0f : 0.0f, DeltaSeconds, 20.0f);

	// Pistol IK 
	bool bPistolIKCondition = bIsPistolEquipped && !bIsSMGEquipped && !bIsBusy && !bFlashlightAimCondition;
	PistolIKAlpha = FMath::FInterpTo(PistolIKAlpha, bPistolIKCondition ? 1.0f : 0.0f, DeltaSeconds, 15.0f);

	// Unarmed FlashLight IK
	float TargetFlashAlpha = (bIsUseFlashLight && !bIsSMGEquipped && !bIsPistolEquipped) ? 1.0f : 0.0f;
	FlashlightAlpha = FMath::FInterpTo(FlashlightAlpha, TargetFlashAlpha, DeltaSeconds, 5.0f);

	// 권총 관절 및 소켓 위치 업데이트
	if (bIsPistolEquipped && WeaponMesh)
	{
		FName TargetSocketName = bIsAiming ? FName("FlashLightIKSocket") : FName("RelaxFlashLightIK_Socket");
		PistolFlashlightIKTargetLoc = WeaponMesh->GetSocketLocation(TargetSocketName);

		FVector TargetJointPos = bIsAiming ? FVector(0.f, -20.f, 0.f) : FVector(0.f, -280.f, -150.f);
		PistolJointTarget = FMath::VInterpTo(PistolJointTarget, TargetJointPos, DeltaSeconds, 15.0f);
	}

	// 아이템 픽업 IK 
	if (APrototypeCharacter* Player = Cast<APrototypeCharacter>(TryGetPawnOwner()))
	{
		PickupTargetLocation = Player->CurrentPickupLocation;

		float PickupCurveValue = GetCurveValue(TEXT("PickupIK"));
		PickupIKAlpha = FMath::FInterpTo(PickupIKAlpha, PickupCurveValue, DeltaSeconds, 15.0f);

		if (PickupIKAlpha > 0.1f)
		{
			DrawDebugSphere(GetWorld(), PickupTargetLocation, 10.f, 12, FColor::Red, false, 0.f);

			// 높이에 따른 Joint Target 계산
			FVector NewJointTarget;
			FVector LocalTargetPos = Player->GetActorTransform().InverseTransformPosition(PickupTargetLocation);

			if (LocalTargetPos.Z < -50.0f) NewJointTarget = FVector(-20.0f, -60.0f, 40.0f);      // 무릎 아래
			else if (LocalTargetPos.Z < 30.0f) NewJointTarget = FVector(-50.0f, -50.0f, 0.0f);   // 허리~가슴
			else NewJointTarget = FVector(-40.0f, -40.0f, -30.0f);                              // 높음

			DynamicPickupJointTarget = FMath::VInterpTo(DynamicPickupJointTarget, NewJointTarget, DeltaSeconds, JointInterpSpeed);
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
		// 인터페이스를 통해 캐릭터의 회복 함수 호출
		AnimInterface->ExecuteHealPoint();
	}
}

void UPlayerAnimInstance::AnimNotify_AttachItem()
{
	if (APrototypeCharacter* Player = Cast<APrototypeCharacter>(TryGetPawnOwner()))
	{
		Player->AttachInteractingItem(); // 아까 만든 부착 함수 호출
	}
}

void UPlayerAnimInstance::AnimNotify_ConsumeItem()
{
	if (APrototypeCharacter* Player = Cast<APrototypeCharacter>(TryGetPawnOwner()))
	{
		Player->ConsumeInteractingItem(); // 여기서 최종 Destroy()
	}
}
