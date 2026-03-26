#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "Character/Components/Interaction/InteractionComponent.h"
#include "Character/Animation/Interface/PlayerAnimInterface.h"
#include "Character/Data/AnimData/CharacterAnimData.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Weapon/Weapon.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimNode_Inertialization.h" 
#include "Gimmic_CY/Object/Door/SingleDoor.h"
#include "Gimmic_CY/Object/ObjectBase.h"
#include "Gimmic_CY/Object/Lever/Lever.h"

void UPlayerAnimInstance::NativeInitializeAnimation() {
	Super::NativeInitializeAnimation();
	Character = Cast<ACharacter>(TryGetPawnOwner());
	CachedCharacter = Cast<APrototypeCharacter>(Character);
	if (Character) MovementComp = Character->GetCharacterMovement();
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds) {
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!Character || !MovementComp || !CachedCharacter) return;

	UpdateEssentialData(DeltaSeconds);
	UpdateCombatIK(DeltaSeconds);
	UpdateInteractionIK(DeltaSeconds);
	UpdateFlashlightLogic(DeltaSeconds);

	if (TurnCooldownTimer > 0.0f) TurnCooldownTimer -= DeltaSeconds;
}

void UPlayerAnimInstance::UpdateEssentialData(float DeltaSeconds) {
	Velocity = Character->GetVelocity();
	Acceleration = MovementComp->GetCurrentAcceleration();

	if (Velocity.SizeSquared() < 1.0f) {
		GroundSpeed = 0.0f; bHasVelocity = false; bIsAcceleration = false;
	}
	else {
		UpdateMovementCalculations(DeltaSeconds);
	}

	if (IPlayerAnimInterface* AnimInterface = Cast<IPlayerAnimInterface>(Character)) {
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
		bIsInteracting = AnimInterface->GetIsInteracting();
		bIsInVent = AnimInterface->GetIsInVent();
		WeaponMesh = AnimInterface->GetEquippedWeaponMesh();
		EquippedWeapon = AnimInterface->GetEquippedWeapon();
		bIsSMGEquipped = AnimInterface->GetIsSMGEquipped();

		AimPitch = bIsReloading ? FMath::FInterpTo(AimPitch, 0.0f, DeltaSeconds, 5.0f) : AnimInterface->GetAimPitch();
	}
}

void UPlayerAnimInstance::UpdateCombatIK(float DeltaSeconds) {
	bool bIsIKBusy = bIsEquipping || bIsReloading || bIsInteracting || bIsQuickTurning;
	float CurveValue = GetCurveValue(TEXT("HandIKLeftAlpha"));
	SMGHandIKAlpha = FMath::FInterpTo(SMGHandIKAlpha, (bIsSMGEquipped && !bIsQuickTurning && !bIsIKBusy) ? CurveValue : 0.0f, DeltaSeconds, 15.0f);
	PistolIKAlpha = FMath::FInterpTo(PistolIKAlpha, (bIsPistolEquipped && !bIsSMGEquipped && !bIsIKBusy) ? 1.0f : 0.0f, DeltaSeconds, 15.0f);

	if (bIsPistolEquipped && WeaponMesh) {
		FName TargetSocket = bIsAiming ? TEXT("FlashLightIKSocket") : TEXT("RelaxFlashLightIK_Socket");
		PistolFlashlightIKTargetLoc = WeaponMesh->GetSocketLocation(TargetSocket);
		FVector TargetJoint = bIsAiming ? FVector(0.f, -20.f, 0.f) : FVector(0.f, -280.f, -150.f);
		PistolJointTarget = FMath::VInterpTo(PistolJointTarget, TargetJoint, DeltaSeconds, 15.0f);
	}
}

void UPlayerAnimInstance::UpdateInteractionIK(float DeltaSeconds) {
	if (!CachedCharacter || !CachedCharacter->InteractionComp) return;

	float PickupCurve = GetCurveValue(TEXT("PickupIK"));
	PickupIKAlpha = FMath::FInterpTo(PickupIKAlpha, PickupCurve, DeltaSeconds, 10.0f);
	if (PickupIKAlpha > 0.1f) {
		PickupTargetLocation = CachedCharacter->InteractionComp->CurrentPickupLocation;
		FVector LocalTarget = CachedCharacter->GetActorTransform().InverseTransformPosition(PickupTargetLocation);
		FVector NewJoint = FVector(-20.f, -40.f, (LocalTarget.Z < -50.0f) ? 20.f : -30.f);
		DynamicPickupJointTarget = FMath::VInterpTo(DynamicPickupJointTarget, NewJoint, DeltaSeconds, 15.0f);
	}

	float LeverCurve = GetCurveValue(TEXT("LeverIK"));
	LeverIKAlpha = FMath::FInterpTo(LeverIKAlpha, LeverCurve, DeltaSeconds, (LeverCurve > LeverIKAlpha) ? 15.0f : 8.0f);
	if (LeverIKAlpha > 0.01f) {
		if (AActor* Item = CachedCharacter->InteractionComp->CurrentInteractingItem) {
			if (AObjectBase* Obj = Cast<AObjectBase>(Item)) {
				if (Obj->PickUpPoint) LeverTargetLocation = Obj->PickUpPoint->GetComponentLocation();
			}
		}
		DynamicLeverJointTarget = FMath::VInterpTo(DynamicLeverJointTarget, FVector(-15.f, -70.f, 0.f), DeltaSeconds, 5.0f);
	}

	if (bIsInteracting) {
		if (ASingleDoor* Door = Cast<ASingleDoor>(CachedCharacter->InteractionComp->CurrentInteractingItem)) {
			FVector HandleLoc = Door->Mesh->GetSocketLocation(TEXT("HandleSocket"));
			PickupTargetLocation = HandleLoc; LeverTargetLocation = HandleLoc;
			LeverTargetRotation = Door->Mesh->GetSocketRotation(TEXT("HandleSocket"));
			PickupIKAlpha = FMath::FInterpTo(PickupIKAlpha, GetCurveValue(TEXT("PickupIK")), DeltaSeconds, 15.0f);
			DynamicPickupJointTarget = (Door->GetSingleDoorAnimationType() == ESingleDoorAnimationType::SingleDoor_Pull) ? FVector(40.f, 80.f, 0.f) : FVector(25.f, 30.f, 0.f);
		}
	}

	if (LeverIKAlpha > 0.1f) DrawDebugSphere(GetWorld(), LeverTargetLocation, 5.0f, 12, FColor::Green, false, -1.f);
	if (PickupIKAlpha > 0.1f) DrawDebugSphere(GetWorld(), PickupTargetLocation, 5.0f, 12, FColor::Blue, false, -1.f);
}

void UPlayerAnimInstance::UpdateFlashlightLogic(float DeltaSeconds) 
{
	// 비무장 상태이면서 레버 상호작용 중인지 
	bool bIsUnarmed = !bIsPistolEquipped && !bIsSMGEquipped; 
	bool bIsLeverInteraction = false;

	if (CachedCharacter && CachedCharacter->InteractionComp && CachedCharacter->InteractionComp->CurrentInteractingItem) 
	{
		if (CachedCharacter->InteractionComp->CurrentInteractingItem->IsA(ALever::StaticClass())) {
			bIsLeverInteraction = true;
		}
	}
	bool bHandIsBusy = bIsInteracting && bIsUnarmed && !bIsLeverInteraction;
	float TargetAlpha = (bIsUseFlashLight && bIsUnarmed && !bIsInVent && !bHandIsBusy) ? 1.0f : 0.0f;
	FlashlightAlpha = FMath::FInterpTo(FlashlightAlpha, TargetAlpha, DeltaSeconds, 5.0f);
}

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds) {
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	if (!Character) return;
	UpdateMovementCalculations(DeltaSeconds);
	UpdateMovementDirection();
	UpdateOrientationWarping(DeltaSeconds);
}

void UPlayerAnimInstance::UpdateMovementCalculations(float DeltaSeconds) {
	GroundSpeed = Velocity.Size2D();
	bHasVelocity = GroundSpeed > (bIsCrouching ? 12.0f : 5.0f);
	bIsAcceleration = (Acceleration * FVector(1.f, 1.f, 0.f)).Size() > 10.0f;
	LocalVelocity2D = Character->GetActorRotation().UnrotateVector(Velocity);
	const FVector DirVector = (GroundSpeed < 15.0f && bIsAcceleration) ? Acceleration : Velocity;
	const FRotator ControlRotYaw = FRotator(0.f, Character->GetControlRotation().Yaw, 0.f);
	BS_Direction = UKismetAnimationLibrary::CalculateDirection(DirVector, ControlRotYaw);
	LocomotionAngle = UKismetAnimationLibrary::CalculateDirection(DirVector, Character->GetActorRotation());
	FallingTime = (!bIsGround) ? FallingTime + DeltaSeconds : 0.0f;
}

void UPlayerAnimInstance::UpdateMovementDirection() {
	if (GroundSpeed < 1.0f) {
		if (FMath::IsWithinInclusive(BS_Direction, -45.f, 45.f)) CurrentDir = ELocomotionDirection::Forward;
		else if (FMath::IsWithinInclusive(BS_Direction, 45.f, 135.f)) CurrentDir = ELocomotionDirection::Right;
		else if (FMath::IsWithinInclusive(BS_Direction, -135.f, -45.f)) CurrentDir = ELocomotionDirection::Left;
		else CurrentDir = ELocomotionDirection::Backward;
		return;
	}
	switch (CurrentDir) {
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

void UPlayerAnimInstance::UpdateOrientationWarping(float DeltaSeconds) {
	float TargetAngle = 0.0f;
	if (bIsInteracting || GroundSpeed < 1.0f) { OrientationWarpingAlpha = FMath::FInterpTo(OrientationWarpingAlpha, 0.0f, DeltaSeconds, 10.0f); return; }
	if (bIsAiming && !bIsRunning) {
		switch (CurrentDir) {
		case ELocomotionDirection::Forward: TargetAngle = 0.0f; break;
		case ELocomotionDirection::Right: TargetAngle = 90.0f; break;
		case ELocomotionDirection::Left: TargetAngle = -90.0f; break;
		case ELocomotionDirection::Backward: TargetAngle = (LocomotionAngle > 0) ? 180.0f : -180.0f; break;
		}
		OrientationWarpingAngle = FRotator::NormalizeAxis(LocomotionAngle - TargetAngle);
	}
	else OrientationWarpingAngle = 0.0f;
	OrientationWarpingAlpha = FMath::FInterpTo(OrientationWarpingAlpha, (bHasVelocity && bIsAiming && !bIsRunning) ? 1.0f : 0.0f, DeltaSeconds, 10.0f);
}

void UPlayerAnimInstance::UpdateLocomotionState(ELocomotionState StateName) { bIsRunning = (StateName == ELocomotionState::Running); }

void UPlayerAnimInstance::RequestLayerInertialBlend(float BlendTime) {
	FInertializationRequest Request;
	Request.Duration = BlendTime;
	UE::Anim::IInertializationRequester* Requester = (UE::Anim::IInertializationRequester*)this;
	if (Requester) Requester->RequestInertialization(Request);
}

void UPlayerAnimInstance::AnimNotify_HealEffect() 
{ 
	if (IPlayerAnimInterface* AnimInterface = Cast<IPlayerAnimInterface>(TryGetPawnOwner())) AnimInterface->ExecuteHealPoint(); 
}
void UPlayerAnimInstance::AnimNotify_AttachItem() 
{ 
	if (CachedCharacter) CachedCharacter->AttachInteractingItem(); 
}
void UPlayerAnimInstance::AnimNotify_ConsumeItem() 
{ 
	if (CachedCharacter) CachedCharacter->ConsumeInteractingItem(); 
}
void UPlayerAnimInstance::AnimNotify_TriggerInteraction() 
{ 
	if (CachedCharacter && CachedCharacter->InteractionComp) CachedCharacter->InteractionComp->TriggerInteraction(); 
}
void UPlayerAnimInstance::AnimNotify_EndInteraction() 
{ 
	if (CachedCharacter && CachedCharacter->InteractionComp) { CachedCharacter->InteractionComp->EndInteraction(); Montage_Stop(0.2f); } 
}
void UPlayerAnimInstance::AnimNotify_FreeMovement() 
{ 
	RootMotionMode = ERootMotionMode::IgnoreRootMotion; 
}
void UPlayerAnimInstance::AnimNotify_HideWeaponForLever() 
{
	if (CachedCharacter && CachedCharacter->CombatComp) {
		bWasWeaponDrawnBeforeLever = CachedCharacter->CombatComp->IsWeaponDrawn();
		if (bWasWeaponDrawnBeforeLever) CachedCharacter->CombatComp->HandleWeaponAttachment(false);
	}
}
void UPlayerAnimInstance::AnimNotify_RestoreWeaponAfterLever() {
	if (CachedCharacter && CachedCharacter->CombatComp && bWasWeaponDrawnBeforeLever) CachedCharacter->CombatComp->HandleWeaponAttachment(true);
}

void UPlayerAnimInstance::CalculateYawDir() 
{ 
	if (Character) RootYawOffset = UKismetMathLibrary::NormalizedDeltaRotator(Character->GetControlRotation(), Character->GetActorRotation()).Yaw; 
}
void UPlayerAnimInstance::HandleTurnning() 
{
	if (bIsTurn || TurnCooldownTimer > 0.0f || bIsInteracting || Montage_IsPlaying(nullptr)) return;
	if (FMath::Abs(RootYawOffset) >= 100.0f && CachedCharacter) {
		bIsTurn = true;
		if (MovementComp) MovementComp->StopMovementImmediately();
		UAnimMontage* MontageToPlay = (RootYawOffset > 0) ? CachedCharacter->AnimData->TurnRight90Montages.FindRef(CachedCharacter->CurrentLayerType) : CachedCharacter->AnimData->TurnLeft90Montages.FindRef(CachedCharacter->CurrentLayerType);
		if (MontageToPlay) Montage_Play(MontageToPlay);
		else bIsTurn = false;
	}
}
void UPlayerAnimInstance::StopTurnIfMove() 
{ 
	if (!bIsTurn) return; bIsTurn = false; if (MovementComp && CachedCharacter && !CachedCharacter->GetIsAiming()) MovementComp->bOrientRotationToMovement = true; Montage_Stop(0.2f); 
}
void UPlayerAnimInstance::AnimNotify_TurnFinished() 
{
	if (CachedCharacter) {
		FRotator FinalRot = CachedCharacter->GetActorRotation();
		FinalRot.Yaw = CachedCharacter->GetControlRotation().Yaw;
		CachedCharacter->SetActorRotation(FinalRot);
		if (MovementComp) MovementComp->RotationRate = FRotator(0.0f, 240.0f, 0.0f);
	}
	bIsTurn = false; TurnCooldownTimer = TurnCooldownDuration; CalculateYawDir();
}