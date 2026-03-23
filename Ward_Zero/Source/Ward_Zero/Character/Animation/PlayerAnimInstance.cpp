#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Animation/Interface/PlayerAnimInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Weapon/Weapon.h"
#include "Animation/AnimNode_Inertialization.h" 
#include "Character/Components/Interaction/InteractionComponent.h"
#include "Gimmic_CY/Interface/InteractionBase.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "Gimmic_CY/Object/ObjectBase.h"
#include "Character/Data/AnimData/CharacterAnimData.h"

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
		bIsAcceleration = false;
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

	if (TurnCooldownTimer > 0.0f)
		TurnCooldownTimer -= DeltaSeconds;

	// IK를 끄는 Busy 조건 
	bool bIsIKBusy = bIsEquipping || bIsReloading || bIsInteracting || bIsQuickTurning;
	/*CalculateYawDir();*/
	if (!bIsIKBusy && bIsGround)
	{
		if (GroundSpeed < 1.0f && !bIsAcceleration)
		{
			HandleTurnning();
		}
		else if (bIsTurn && bIsAcceleration)
		{
			StopTurnIfMove();
		}
	}
	else if (bIsTurn)
	{
		StopTurnIfMove(); // 상태가 변하면 턴 강제 종료
	}

	// SMG IK
	float CurveValue = GetCurveValue(TEXT("HandIKLeftAlpha"));
	bool bCanUseSMGIK = bIsSMGEquipped && !bIsQuickTurning;
	SMGHandIKAlpha = FMath::FInterpTo(SMGHandIKAlpha, (bCanUseSMGIK && !bIsIKBusy) ? CurveValue : 0.0f, DeltaSeconds, 15.0f);

	// Pistol IK 조건
	bool bPistolIKCondition = bIsPistolEquipped && !bIsSMGEquipped && !bIsIKBusy;
	PistolIKAlpha = FMath::FInterpTo(PistolIKAlpha, bPistolIKCondition ? 1.0f : 0.0f, DeltaSeconds, 15.0f);

	// Flashlight IK
	// 조준 중 손전등 IK 
	bool bFlashlightAimCondition = bIsUseFlashLight && bIsPistolEquipped && bIsAiming && !bIsSMGEquipped && !bIsIKBusy;
	FlashlightAimIKAlpha = FMath::FInterpTo(FlashlightAimIKAlpha, 0.0f, DeltaSeconds, 20.0f);

	// Flashlight Alpha 
	float TargetFlashAlpha = 0.0f;
	if (bIsUseFlashLight && !bIsSMGEquipped)
	{
		// 비무장 상태(둘 다 false)이거나, 권총 조준 중일 때 켬
		if ((!bIsPistolEquipped && !bIsSMGEquipped) || bIsAiming)
		{
			TargetFlashAlpha = 1.0f;
		}
	}
	FlashlightAlpha = FMath::FInterpTo(FlashlightAlpha, TargetFlashAlpha, DeltaSeconds, 5.0f);
	
	// 픽업 및 오브젝트 소켓 및 관절 위치 업데이트 
	if (bIsPistolEquipped && WeaponMesh)
	{
		FName TargetSocketName = bIsAiming ? FName("FlashLightIKSocket") : FName("RelaxFlashLightIK_Socket");
		PistolFlashlightIKTargetLoc = WeaponMesh->GetSocketLocation(TargetSocketName);

		FVector TargetJointPos = bIsAiming ? FVector(0.f, -20.f, 0.f) : FVector(0.f, -280.f, -150.f);
		PistolJointTarget = FMath::VInterpTo(PistolJointTarget, TargetJointPos, DeltaSeconds, 15.0f);
	}
	if (CachedCharacter)
	{
		// 아이템 픽업 IK 
		// PickupIK 커브가 있을 때만 작동
		if (CachedCharacter && CachedCharacter->InteractionComp)
		{
			PickupTargetLocation = CachedCharacter->InteractionComp->CurrentPickupLocation;
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
		// 레버 IK 
		// LeverIK 커브가 있을 때만 작동
		float LeverCurveValue = GetCurveValue(TEXT("LeverIK"));

		if (LeverCurveValue > LeverIKAlpha)
		{
			LeverIKAlpha = FMath::FInterpTo(LeverIKAlpha, LeverCurveValue, DeltaSeconds, 15.0f);
		}
		else
		{
			// 내려갈 때(0이 될 때) 부드럽게 풀리도록 설정 (튕김 방지)
			LeverIKAlpha = FMath::FInterpTo(LeverIKAlpha, LeverCurveValue, DeltaSeconds, 8.0f);
		}

		if (LeverIKAlpha > 0.01f)
		{
			if (CachedCharacter && CachedCharacter->InteractionComp && CachedCharacter->InteractionComp->CurrentInteractingItem)
			{
				AActor* InteractItem = CachedCharacter->InteractionComp->CurrentInteractingItem;
				if (IsValid(InteractItem) && InteractItem->GetClass()->ImplementsInterface(UInteractionBase::StaticClass()))
				{
					// 매 프레임 레버의 PickupPoint(현재 위치)를 받아와서 타겟으로 설정합니다.
					if (AObjectBase* ObjectBase = Cast<AObjectBase>(InteractItem))
					{
						if (ObjectBase->PickUpPoint)
						{
							// 직접 컴포넌트의 월드 좌표를 가져옵니다.
							LeverTargetLocation = ObjectBase->PickUpPoint->GetComponentLocation();
						}
					}
				}
			}

			// 조인트(팔꿈치) 위치 업데이트
			FVector NewLeverJointTarget = FVector(-15.f, 30.f, 0.f);
			DynamicLeverJointTarget = FMath::VInterpTo(DynamicLeverJointTarget, NewLeverJointTarget, DeltaSeconds, 5.0f);
		}
	}
	if (Character)
	{
		// 캐릭터(캡슐)의 현재 Yaw 각도를 실시간으로 화면 파란색으로 띄움
		if (GEngine) GEngine->AddOnScreenDebugMessage(5, 0.0f, FColor::Cyan, FString::Printf(TEXT("Actor Yaw: %f"), Character->GetActorRotation().Yaw));
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

	if (GroundSpeed < 1.0f)
	{
		OrientationWarpingAngle = 0.0f;
		OrientationWarpingAlpha = FMath::FInterpTo(OrientationWarpingAlpha, 0.0f, DeltaSeconds, 10.0f);
		return;
	}

	// 조준 중(bIsAiming)이고 달리기가 아닐 때만 정교한 방향 계산 수행
	if (bIsAiming && !bIsRunning)
	{
		switch (CurrentDir)
		{
		case ELocomotionDirection::Forward:  TargetAngle = 0.0f; break;
		case ELocomotionDirection::Right:    TargetAngle = 90.0f; break;
		case ELocomotionDirection::Left:     TargetAngle = -90.0f; break;
		case ELocomotionDirection::Backward: TargetAngle = (LocomotionAngle > 0) ? 180.0f : -180.0f; break;
		}
		OrientationWarpingAngle = FRotator::NormalizeAxis(LocomotionAngle - TargetAngle);
	}
	else
	{
		OrientationWarpingAngle = 0.0f;
	}
	const float TargetAlpha = (bHasVelocity && bIsAiming && !bIsRunning) ? 1.0f : 0.0f;
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

void UPlayerAnimInstance::AnimNotify_HideWeaponForLever()
{
	if (CachedCharacter && CachedCharacter->CombatComp)
	{
		bWasWeaponDrawnBeforeLever = CachedCharacter->CombatComp->IsWeaponDrawn();

		// 무기를 들고 있다면 잠시 등에 멘 상태(or 홀스터)로 변경
		if (CachedCharacter->CombatComp->IsWeaponDrawn())
		{
			CachedCharacter->CombatComp->HandleWeaponAttachment(false);
		}
	}
}

void UPlayerAnimInstance::AnimNotify_RestoreWeaponAfterLever()
{
	if (CachedCharacter && CachedCharacter->CombatComp)
	{
		if (bWasWeaponDrawnBeforeLever)
		{
			CachedCharacter->CombatComp->HandleWeaponAttachment(true);
			bWasWeaponDrawnBeforeLever = false; // 상태 초기화
		}
	}
}

void UPlayerAnimInstance::AnimNotify_TriggerInteraction()
{
	if (CachedCharacter && CachedCharacter->InteractionComp)
	{
		CachedCharacter->InteractionComp->TriggerInteraction();
	}
}

void UPlayerAnimInstance::AnimNotify_EndInteraction()
{
	if (CachedCharacter && CachedCharacter->InteractionComp)
	{
		CachedCharacter->InteractionComp->EndInteraction();
		CachedCharacter->InteractionComp->bIsInteractingDoor = false;

		if (APlayerController* PC = Cast<APlayerController>(CachedCharacter->GetController()))
		{
			CachedCharacter->EnableInput(PC);
		}
		Montage_Stop(0.2f);
	}
}

void UPlayerAnimInstance::AnimNotify_FreeMovement()
{
	RootMotionMode = ERootMotionMode::IgnoreRootMotion;
}

void UPlayerAnimInstance::CalculateYawDir()
{
	if (!Character) return;

	FRotator ControlRot = Character->GetControlRotation();
	FRotator ActorRot = Character->GetActorRotation();

	// 현재 컨트롤 회전(카메라)과 액터 회전의 차이 (Yaw) 계산
	RootYawOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot).Yaw;
}

void UPlayerAnimInstance::HandleTurnning()
{
	// 이미 턴 중이거나 쿨타임이면 절대 진입 금지
	if (bIsTurn || TurnCooldownTimer > 0.0f) return;

	// 슬롯 설정 문제로 모션이 안 보일 수 있으므로 재생 여부 재체크
	if (Montage_IsPlaying(nullptr)) return;

	// [수정] 90도 루트모션이라면 임계값을 약간 여유 있게(약 100도) 설정
	if (FMath::Abs(RootYawOffset) >= 100.0f)
	{
		if (CachedCharacter)
		{
			bIsTurn = true; // 가장 먼저 플래그를 세워 중복 실행 방지

			EWeaponLayerType LayerType = CachedCharacter->CurrentLayerType;
			UAnimMontage* MontageToPlay = (RootYawOffset > 0) ?
				CachedCharacter->AnimData->TurnRight90Montages.FindRef(LayerType) :
				CachedCharacter->AnimData->TurnLeft90Montages.FindRef(LayerType);

			if (MontageToPlay)
			{
				// 재생 전 캐릭터의 회전 가속도를 0으로 밀어버림
				CachedCharacter->GetCharacterMovement()->StopMovementImmediately();
				Montage_Play(MontageToPlay);
			}
			else
			{
				bIsTurn = false;
			}
		}
	}
}

void UPlayerAnimInstance::StopTurnIfMove()
{
	if (!bIsTurn) return;
	bIsTurn = false;

	if (MovementComp && CachedCharacter && !CachedCharacter->GetIsAiming())
	{
		MovementComp->bOrientRotationToMovement = true;
	}

	// 이동 시 재생 중이던 턴 몽타주를 부드럽게 블렌드 아웃(0.2초)하여 이동 상태로 즉시 전환
	if (CachedCharacter && CachedCharacter->AnimData)
	{
		EWeaponLayerType LayerType = CachedCharacter->CurrentLayerType;
		UAnimMontage* LeftMontage = CachedCharacter->AnimData->TurnLeft90Montages.FindRef(LayerType);
		UAnimMontage* RightMontage = CachedCharacter->AnimData->TurnRight90Montages.FindRef(LayerType);

		if (LeftMontage && Montage_IsPlaying(LeftMontage)) Montage_Stop(0.2f, LeftMontage);
		if (RightMontage && Montage_IsPlaying(RightMontage)) Montage_Stop(0.2f, RightMontage);
	}
}

void UPlayerAnimInstance::AnimNotify_TurnFinished()
{
	if (CachedCharacter)
	{
		// [수정] 현재 카메라가 보고 있는 정확한 Yaw 값을 가져옴
		float TargetYaw = CachedCharacter->GetControlRotation().Yaw;
		FRotator FinalRot = CachedCharacter->GetActorRotation();
		FinalRot.Yaw = TargetYaw;

		// 캐릭터를 카메라 방향으로 강제 순간이동(회전) 시킴
		CachedCharacter->SetActorRotation(FinalRot);

		// 회전 속도 복구
		CachedCharacter->GetCharacterMovement()->RotationRate = FRotator(0.0f, 240.0f, 0.0f);
	}

	bIsTurn = false;
	TurnCooldownTimer = TurnCooldownDuration;

	// 종료 후 오프셋 즉시 재계산 (0에 가깝게 나옴)
	CalculateYawDir();
}