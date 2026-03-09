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

	Velocity = Character->GetVelocity();
	Acceleration = MovementComp->GetCurrentAcceleration();
	UpdateMovementCalculations(DeltaSeconds);

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

		USkeletalMeshComponent* TempMesh = AnimInterface->GetEquippedWeaponMesh();
		WeaponMesh = IsValid(TempMesh) ? TempMesh : nullptr;

		AWeapon* TempWeapon = AnimInterface->GetEquippedWeapon();
		EquippedWeapon = IsValid(TempWeapon) ? TempWeapon : nullptr;

		bIsSMGEquipped = AnimInterface->GetIsSMGEquipped();

		if (bIsReloading)
		{
			AimPitch = FMath::FInterpTo(AimPitch, 0.0f, DeltaSeconds, 5.0f);
		}
		else
		{
			AimPitch = AnimInterface->GetAimPitch();
		}
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
	// Pistol IK 조건  
	bool bPistolIKCondition = bIsPistolEquipped && !bIsSMGEquipped
		&& !bIsEquipping && !bIsReloading && !bIsInteracting
		&& !bIsUseFlashLight;

	PistolIKAlpha = FMath::FInterpTo(PistolIKAlpha, bPistolIKCondition ? 1.0f : 0.0f, DeltaSeconds, 15.0f);

	// SMG IK 조건 
	bool bIKCondition =
		bIsSMGEquipped &&
		!bIsReloading &&
		!bIsEquipping &&
		!bIsQuickTurning &&
		!bIsInteracting;

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

	//Unarmed FlashLight IK Alpha
	float TargetAlpha = (bIsUseFlashLight && !bIsSMGEquipped) ? 1.0f : 0.0f;
	FlashlightAlpha = FMath::FInterpTo(FlashlightAlpha, TargetAlpha, DeltaSeconds, 5.0f);

	//Pistol FlashLight IK Alpha
	bool bFlashlightIKCondition = bIsUseFlashLight && bIsPistolEquipped && !bIsSMGEquipped
		&& !bIsRunning && (GroundSpeed <= 250.f)
		&& !bIsEquipping && !bIsReloading && !bIsInteracting;

	FlashlightIKAlpha = FMath::FInterpTo(FlashlightIKAlpha, bFlashlightIKCondition ? 1.0f : 0.0f, DeltaSeconds, 20.0f);
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