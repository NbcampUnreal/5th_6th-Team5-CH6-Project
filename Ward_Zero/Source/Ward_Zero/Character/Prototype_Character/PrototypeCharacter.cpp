#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/PlayerStatusComponent.h"
#include "Character/Components/PlayerCombatComponent.h"
#include "Character/Components/InteractionComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Objects/Ladder.h"
#include "Objects/Interface/Interact.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Character/Animation/PlayerAnimInstance.h"
#include "Engine/Engine.h"
#include "Weapon/WZ_HUD_DH.h"
#include "Weapon/Weapon.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "FlashLight/FlashLight.h"
#include "Gimmic_CY/InteractionBase.h"
#include "Character/Data/CharacterMovementData.h"
#include "Character/Data/CharacterStatusData.h"
#include "Weapon/Data/WeaponData.h"
#include "Character/Data/CharacterAnimData.h"

APrototypeCharacter::APrototypeCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 컴포넌트 생성
	StatusComponent = CreateDefaultSubobject<UPlayerStatusComponent>(TEXT("StatusComp"));
	CombatComponent = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("CombatComp"));
	InteractionComp = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComp"));

	// 캐릭터 회전 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 캐릭터 무브먼트 설정
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 240.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.f;
	GetCharacterMovement()->MaxAcceleration = 800.f;
	GetCharacterMovement()->GroundFriction = 6.f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchMovementSpeed;

	// 카메라 붐 생성 및 설정
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	CameraBoom->TargetArmLength = StandingArmLength;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 15.0f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 25.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 35.0f, 10.0f);
	CameraBoom->ProbeSize = 12.0f;

	// 카메라 생성 및 설정
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	MainCamera->bUsePawnControlRotation = false;
	MainCamera->FieldOfView = 60.0f;  

	// 캡슐과 메쉬가 카메라를 막지 않도록 설정
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	Tags.Add(TEXT("Player"));
}

void APrototypeCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!MovementData || !StatusData) return; 

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}

		if (PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->ViewPitchMin = -60.0f;
			PlayerController->PlayerCameraManager->ViewPitchMax = 50.0f;
		}
	}
	// MovementData 적용(속도 및 카메라 초기화)
	if (MovementData && GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = MovementData->WalkSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = MovementData->CrouchMovementSpeed;

		if (CameraBoom)
		{
			CameraBoom->TargetArmLength = MovementData->DefaultArmLength;
			StandingArmLength = MovementData->DefaultArmLength;
			AimArmLength = MovementData->AimArmLength;
			PistolAimSocketOffset = MovementData->PistolAimSocketOffset;
		}
	}
	// 카메라 기본값 백업
	if (CameraBoom && MainCamera)
	{
		OriginalTargetOffset = CameraBoom->TargetOffset;
		OriginalSocketOffset = CameraBoom->SocketOffset;
		OriginalFOV = MainCamera->FieldOfView;
	}
	// StatusData 적용 (체력 및 스테미나 초기화)
	if (StatusData && StatusComponent)
	{
		StatusComponent->MaxHealth = StatusData->MaxHealth;
		StatusComponent->CurrHealth = StatusData->MaxHealth; 
		StatusComponent->MaxStamina = StatusData->MaxStamina;
		StatusComponent->CurrStamina = StatusData->MaxStamina;
		StatusComponent->StaminaDrainRate = StatusData->StaminaDrainRate;

		StatusComponent->OnPlayerDied.AddDynamic(this, &APrototypeCharacter::OnDeath);
	}

	if (CombatComponent)
	{
		CombatComponent->SetupCombat(MainCamera);
	}

	UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (IsValid(AnimInst) && UnarmedLayerClass)
	{
		AnimInst->LinkAnimClassLayers(UnarmedLayerClass);
	}
}

void APrototypeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CameraBoom || !MainCamera || !GetCharacterMovement()) return;

	CheckRunState();

	if (FlashLight)
	{
		// 장전 중이거나, 무기를 장착/해제 몽타주 호출 시 손전등 숨김. 
		bool bShouldHide = GetIsReloading() || IsEquipping();
		FlashLight->SetActorHiddenInGame(bShouldHide);
	}

	if (bIsQuickTurning)
	{
		float SafeDuration = (TurnDuration > KINDA_SMALL_NUMBER) ? TurnDuration : 1.0f;
		TurnAlpha += DeltaTime / SafeDuration;

		if (TurnAlpha >= 1.0f)
		{
			// 턴 종료 시 Actor 회전 고정
			FRotator FinalRot = GetActorRotation();
			FinalRot.Yaw = FRotator::NormalizeAxis(TurnStartYaw + TurnYawDelta);
			SetActorRotation(FinalRot, ETeleportType::TeleportPhysics);

			//턴 종료 시 컨트롤러(카메라) 회전 고정
			if (Controller)
			{
				FRotator FinalControlRot = Controller->GetControlRotation();
				FinalControlRot.Yaw = FRotator::NormalizeAxis(ControlStartYaw + TurnYawDelta);
				Controller->SetControlRotation(FinalControlRot);
			}

			StopQuickTurn();
		}
		else
	{
		float SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, TurnAlpha, 2.0f);

		//캐릭터 몸 회전
		float CurrentYaw = TurnStartYaw + (TurnYawDelta * SmoothAlpha);
		FRotator NewRot = GetActorRotation();
		NewRot.Yaw = CurrentYaw;
		SetActorRotation(NewRot, ETeleportType::None);

		//카메라 회전 
		if (Controller)
		{
			float CurrentControlYaw = ControlStartYaw + (TurnYawDelta * SmoothAlpha);
			FRotator NewControlRot = Controller->GetControlRotation();
			NewControlRot.Yaw = CurrentControlYaw;
			Controller->SetControlRotation(NewControlRot);
		}
	}
		return;
	}

	if (!bIsQuickTurning)
	{
		if (CombatComponent && CombatComponent->IsAiming())
		{
			// 조준 중일 때
			FRotator NewRot = GetActorRotation();
			NewRot.Yaw = GetControlRotation().Yaw;
			SetActorRotation(NewRot);
		}
		else
		{
			// 평상시 (조준 안 할 때)
			bUseControllerRotationYaw = false;

			if (bIsRunning)
			{
				GetCharacterMovement()->bOrientRotationToMovement = true;
			}
			else
			{
				GetCharacterMovement()->bOrientRotationToMovement = false;

				float CurrentSpeed = GetVelocity().SizeSquared();
				if (CurrentSpeed > 10.0f)
				{
					FRotator TargetRotation = FRotator(0.0f, GetControlRotation().Yaw, 0.0f);
					FRotator CurrentRotation = GetActorRotation();
					FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f);
					SetActorRotation(NewRotation);
				}
			}
		}
	}

	if (GEngine)
	{
		if (StatusComponent)
		{
			FString StatusMsg = FString::Printf(TEXT("HP: %.0f / Stamina: %.0f"),
				StatusComponent->CurrHealth, StatusComponent->CurrStamina);

			GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Green, StatusMsg);
		}

		if (CombatComponent && CombatComponent->EquippedWeapon)
		{
			FString AmmoMsg = FString::Printf(TEXT("Ammo: %d / %d"),
				CombatComponent->EquippedWeapon->GetCurrentAmmo(),
				CombatComponent->EquippedWeapon->GetReserveAmmo());

			GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::Yellow, AmmoMsg);
		}
	}

#pragma region Camera Aim & Bobbing

	float TargetArmLengthDest;
	float TargetFOVDest;
	FVector TargetSocketOffsetDest;
	FVector TargetTargetOffsetDest;

	if (CombatComponent && CombatComponent->IsAiming())
	{
		TargetArmLengthDest = AimArmLength;
		TargetFOVDest = AimFOV;
		TargetTargetOffsetDest = FVector::ZeroVector;

		if (CombatComponent->GetCurrentWeaponIndex() == 2)
		{
			TargetSocketOffsetDest = SMGAimSocketOffset; // SMG 들고 있을 때
		}
		else
		{
			TargetSocketOffsetDest = PistolAimSocketOffset; // 권총 들고 있을 때
		}

		float CurrentTime = GetWorld()->GetTimeSeconds();

		float BreathDeltaYaw = 0.6f * 1.2f * FMath::Cos(1.2f * CurrentTime) * DeltaTime;
		float BreathDeltaPitch = -0.6f * 2.4f * FMath::Sin(2.4f * CurrentTime) * DeltaTime;

		float TotalDeltaYaw = BreathDeltaYaw;
		float TotalDeltaPitch = BreathDeltaPitch;

		float Speed = GetVelocity().Size();
		if (Speed > 10.0f && GetCharacterMovement()->IsMovingOnGround())
		{
			float WalkDeltaYaw = 0.4f * 6.0f * FMath::Cos(6.0f * CurrentTime) * DeltaTime;
			float WalkDeltaPitch = -0.4f * 12.0f * FMath::Sin(12.0f * CurrentTime) * DeltaTime;

			TotalDeltaYaw += WalkDeltaYaw;
			TotalDeltaPitch += WalkDeltaPitch;
		}

		bool bShouldSway = true;
		if (CombatComponent && (CombatComponent->GetIsReloading() || CombatComponent->IsFiring()))
		{
			bShouldSway = false;
		}

		if (Controller && bShouldSway)
		{
			FRotator CurrentControlRot = Controller->GetControlRotation();
			CurrentControlRot.Yaw += TotalDeltaYaw;
			CurrentControlRot.Pitch += TotalDeltaPitch;
			Controller->SetControlRotation(CurrentControlRot);
		}
	}
	else
	{
		TargetArmLengthDest = bIsCrouched ? CrouchedArmLength : OriginalArmLength;
		TargetFOVDest = OriginalFOV;
		TargetTargetOffsetDest = OriginalTargetOffset;

		FVector Velocity = GetVelocity();
		float Speed = Velocity.Size();
		float ZOffsetBob = 0.0f;
		float YOffsetBob = 0.0f;

		if (Speed > 10.0f && GetCharacterMovement()->IsMovingOnGround())
		{
			float ActualFrequency;
			float ActualAmplitude;
			float SpeedDivider;

			if (bIsRunning)
			{
				SpeedDivider = 300.0f;

				ActualFrequency = BobFrequency * 0.8f;

				ActualAmplitude = BobAmplitude * 1.5f;
			}
			else
			{
				SpeedDivider = 150.0f;
				ActualFrequency = BobFrequency;
				ActualAmplitude = BobAmplitude;
			}

			if (SpeedDivider < KINDA_SMALL_NUMBER) SpeedDivider = 150.0f; // 0 방지

			BobTime += DeltaTime * (Speed / SpeedDivider) * ActualFrequency;

			ZOffsetBob = FMath::Sin(BobTime) * ActualAmplitude;

			float HorizontalScale = bIsRunning ? 1.5f : 1.0f;
			YOffsetBob = FMath::Cos(BobTime * 0.5f) * BobHorizontalAmplitude * HorizontalScale;

		}
		else
		{
			BobTime = 0.0f;
		}

		TargetSocketOffsetDest = OriginalSocketOffset;

		if (bIsCrouched)
		{
			if (Speed > 10.0f && GetCharacterMovement()->IsMovingOnGround())
			{
				TargetSocketOffsetDest.Z = CrouchedWalkCameraHeight;
			}
			else
			{
				TargetSocketOffsetDest.Z = CrouchedCameraHeight;
			}
		}

		TargetSocketOffsetDest.Y += YOffsetBob;
		TargetSocketOffsetDest.Z += ZOffsetBob;
		
	}

	float InterpSpeed = AimInterpSpeed;

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLengthDest, DeltaTime, InterpSpeed);
	MainCamera->FieldOfView = FMath::FInterpTo(MainCamera->FieldOfView, TargetFOVDest, DeltaTime, InterpSpeed);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffsetDest, DeltaTime, InterpSpeed);
	CameraBoom->TargetOffset = FMath::VInterpTo(CameraBoom->TargetOffset, TargetTargetOffsetDest, DeltaTime, InterpSpeed);
#pragma endregion
}

void APrototypeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 이동 및 움직임 입력 바인딩
		if (MoveAction) EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Move);
		if (LookAction) EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Look);
		if (RunAction) EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &APrototypeCharacter::StartRunning);
		if (CrouchAction) EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &APrototypeCharacter::ToggleCrouch);
		if (QuickTurnAction) EnhancedInputComponent->BindAction(QuickTurnAction, ETriggerEvent::Started, this, &APrototypeCharacter::PerformQuickTurn180);

		// 상호작용 입력 바인딩
		if (InteractAction) EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APrototypeCharacter::Interact);

		// 전투 입력 바인딩
		if (EquipAction) EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &APrototypeCharacter::ToggleEquip);
		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APrototypeCharacter::StartAiming);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APrototypeCharacter::StopAiming);
		}
		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APrototypeCharacter::Fire);
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &APrototypeCharacter::StopFire);
		}
		if (ReloadAction) EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &APrototypeCharacter::Reload);		
		if (EquipSlot1Action) EnhancedInputComponent->BindAction(EquipSlot1Action, ETriggerEvent::Started, this, &APrototypeCharacter::SelectWeapon1);
		if (EquipSlot2Action) EnhancedInputComponent->BindAction(EquipSlot2Action, ETriggerEvent::Started, this, &APrototypeCharacter::SelectWeapon2);

		// 아이템 사용 입력 바인딩
		if (FlashLightAction) EnhancedInputComponent->BindAction(FlashLightAction, ETriggerEvent::Started, this, &APrototypeCharacter::ToggleFlashLight);
	}
}

// 데미지 처리
float APrototypeCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// StatusComponent에게 생존 여부 확인
	if (!StatusComponent || StatusComponent->IsDead()) return 0.0f;

	// 데미지 계산을 위임하고 실제 입은 피해량 받기
	float ActualDamage = StatusComponent->ApplyDamage(DamageAmount);

	// 피격 방향 계산
	FVector ToAttackerDir = FVector::ZeroVector;
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
		ToAttackerDir = -PointEvent->ShotDirection;
	}
	else if (DamageCauser)
	{
		ToAttackerDir = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	}

	// 살아있으면 피격 모션, 죽었으면 OnDeath 델리게이트가 처리
	if (!StatusComponent->IsDead())
	{
		PlayHitReaction(ToAttackerDir);
	}

	return ActualDamage;
}

// StatusComponent에서 사망 신호가 오면 실행됨
void APrototypeCharacter::OnDeath()
{
	// 래그돌 처리 및 입력 차단
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		DisableInput(PC);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);
}

EPlayerHitDirection APrototypeCharacter::GetHitDirection(const FVector& ToAttackerDir)
{
	FVector ToAttacker = ToAttackerDir;
	FVector MyForward = GetActorForwardVector();
	FVector MyRight = GetActorRightVector();

	float ForwardDot = FVector::DotProduct(MyForward, ToAttacker);
	float RightDot = FVector::DotProduct(MyRight, ToAttacker);

	if (ForwardDot >= 0.5f) return EPlayerHitDirection::Front;
	else if (ForwardDot <= -0.5f) return EPlayerHitDirection::Back;
	else return (RightDot > 0.f) ? EPlayerHitDirection::Right : EPlayerHitDirection::Left;
}

void APrototypeCharacter::StartQuickTurn(float TargetYawDelta)
{
	if (bIsQuickTurning || bIsRunning || bIsClimbing) return;
	if (CombatComponent && CombatComponent->IsAiming()) return;

	bIsQuickTurning = true;
	TurnAlpha = 0.f;
	TurnStartYaw = GetActorRotation().Yaw;
	TurnYawDelta = TargetYawDelta;

	if (Controller)
	{
		ControlStartYaw = Controller->GetControlRotation().Yaw;
	}

	bool bHasPistol = CombatComponent && CombatComponent->IsPistolEquipped();

	// 회전 각도와 무기 유무에 따른 인덱스 및 시간 세팅
	if (FMath::Abs(TargetYawDelta) > 100.0f) // 180도 턴
	{
		TurnIndex = bHasPistol ? 6 : 2; // Pistol 180 : Unarmed 180
		TurnDuration = Duration180;
	}
	else if (TargetYawDelta > 0) // 오른쪽 90도
	{
		TurnIndex = bHasPistol ? 8 : 4; // Pistol R90 : Unarmed R90
		TurnDuration = Duration90;
	}
	else // 왼쪽 90도
	{
		TurnIndex = bHasPistol ? 7 : 3; // Pistol L90 : Unarmed L90
		TurnDuration = Duration90;
	}

	// 물리 이동 일시 정지 
	GetCharacterMovement()->StopMovementImmediately();
}

void APrototypeCharacter::StopQuickTurn()
{
	bIsQuickTurning = false;
	TurnAlpha = 0.f;
	TurnIndex = 0;

	// 조준 상태였다면 다시 컨트롤러 회전 복구
	if (CombatComponent && CombatComponent->IsAiming())
	{
		bUseControllerRotationYaw = true;
	}
}

void APrototypeCharacter::PerformQuickTurn180()
{
	StartQuickTurn(180.0f);
}

void APrototypeCharacter::PerformQuickTurn90(float Angle)
{
}

void APrototypeCharacter::ProcessMovementTurn(FVector2D MovementVector)
{
}

void APrototypeCharacter::PlayHitReaction(const FVector& ToAttackerDir)
{
	if (bIsQuickTurning) StopQuickTurn();
	if (CombatComponent && CombatComponent->IsAiming()) CombatComponent->StopAiming();

	// 방향 계산 (Front, Back, Left, Right 반환)
	EPlayerHitDirection HitDir = GetHitDirection(ToAttackerDir);

	// AnimData 에셋의 TMap에서 몽타주 찾기
	if (AnimData && AnimData->HitMontages.Contains(HitDir))
	{
		UAnimMontage* MontageToPlay = AnimData->HitMontages[HitDir];

		UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
		if (AnimInst && MontageToPlay)
		{
			AnimInst->Montage_Play(MontageToPlay);
		}
	}
}

void APrototypeCharacter::PlayDeathReaction(const FVector& ToAttackerDir)
{
	// 방향 계산
	EPlayerHitDirection HitDir = GetHitDirection(ToAttackerDir);

	UAnimMontage* MontageToPlay = nullptr;

	// AnimData 에셋에서 해당 방향의 사망 몽타주 찾기
	if (AnimData && AnimData->DeathMontages.Contains(HitDir))
	{
		MontageToPlay = AnimData->DeathMontages[HitDir];
	}

	if (MontageToPlay)
	{
		// 몽타주 재생 
		PlayAnimMontage(MontageToPlay);
	}
	else
	{
		// 몽타주가 설정 안 되어 있다면 물리 엔진(래그돌) 
		GetMesh()->SetSimulatePhysics(true);
	}
}

void APrototypeCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller == nullptr || bIsQuickTurning) return;

	if (bIsClimbing)
	{
		AddMovementInput(FVector::UpVector, MovementVector.Y);
		return;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void APrototypeCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		float CurrentSensitivity = (CombatComponent && CombatComponent->IsAiming()) ? AimLookSensitivity : 1.0f;

		AddControllerYawInput(LookAxisVector.X * CurrentSensitivity);
		AddControllerPitchInput(LookAxisVector.Y * CurrentSensitivity);
	}
}

void APrototypeCharacter::ToggleCrouch(const FInputActionValue& Value)
{
	if (bIsRunning || bIsClimbing) return;
	if (bIsCrouched) UnCrouch();
	else Crouch();
}

void APrototypeCharacter::StartRunning(const FInputActionValue& Value)
{
	if (CombatComponent && CombatComponent->IsAiming()) return;

	if (bIsCrouched || bIsClimbing) return;

	if (StatusComponent && !StatusComponent->CanSprint()) return;

	if (bIsRunning)
	{
		EndRunning(Value);
	}
	else
	{
		bIsRunning = true;
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		CameraBoom->CameraLagSpeed = 10.0f;
	}
}

void APrototypeCharacter::EndRunning(const FInputActionValue& Value)
{
	UPlayerAnimInstance* AnimInst = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());

	if (AnimInst)
	{
		AnimInst->UpdateLocomotionState(ELocomotionState::Walking);
	}

	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = false; // Strafe 모드로 복구
	CameraBoom->CameraLagSpeed = 15.0f;
}

void APrototypeCharacter::CheckRunState()
{
	if (bIsRunning)
	{
		if (bIsRunning)
		{
			if(StatusComponent && !StatusComponent->CanSprint())
			{
				EndRunning(FInputActionValue());
				return;
			}

			float CurrentSpeed = GetVelocity().Size();
			if (CurrentSpeed <= KINDA_SMALL_NUMBER)
			{
				EndRunning(FInputActionValue());
			}
		}
	}
}

void APrototypeCharacter::Reload(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

void APrototypeCharacter::ToggleFlashLight(const FInputActionValue& Value)
{
	if (GetIsSMGEquipped()) return;

	bIsUseFlashLight = !bIsUseFlashLight;
	
	ToggleLight(bIsUseFlashLight);
}

void APrototypeCharacter::Interact(const FInputActionValue& Value)
{
	if (InteractionComp)
	{
		InteractionComp->TryInteract();
	}
	TArray<AActor*> OverlappedActors;
	GetCapsuleComponent()->GetOverlappingActors(OverlappedActors);

	for (AActor* OverlappedActor : OverlappedActors)
	{
		if (OverlappedActor->GetClass()->ImplementsInterface(UInteractionBase::StaticClass()))
		{
			if (IInteractionBase* InteractbalesInterface = Cast<IInteractionBase>(OverlappedActor))
			{
				InteractbalesInterface->OnIneracted(this);
				break;
			}
		}
	}
}

void APrototypeCharacter::StartClimbing(ALadder* Ladder)
{
	
}

void APrototypeCharacter::StopClimbing()
{
	bIsClimbing = false;
	CurrentLadder = nullptr;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APrototypeCharacter::EquipFlashLight()
{
	if (FlashLightClass)
	{
		FlashLight = GetWorld()->SpawnActor<AFlashLight>(FlashLightClass);

		// 무기장착여부에 따른 소켓 결정 
		FName SocketName = GetIsPistolEquipped() ? TEXT("FlashLightSocket_Pistol") : TEXT("FlashLightSocket_Normal");

		// 손전등 부착 
		FlashLight->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
		FlashLight->SetActorEnableCollision(false);
	}

	UPlayerAnimInstance* AnimInst = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (AnimInst)
	{
		AnimInst->bIsMirroring = true;
		PlayAnimMontage(RaiseLight);
	}
}

void APrototypeCharacter::ToggleLight(bool IsLight)
{
	UPlayerAnimInstance* AnimInst = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (!AnimInst) return;

	//손전등 ON 
	if (IsLight)
	{
		if (FlashLight == nullptr)
		{
			EquipFlashLight(); 
		}
	}//손전등 OFF
	else
	{
		if (FlashLight)
		{
			PlayAnimMontage(LowerLight); // 집어넣는 동작 재생
			FlashLight->Destroy();
			FlashLight = nullptr;
		}
	}

}

bool APrototypeCharacter::GetIsUseFlashLight() const
{
	return bIsUseFlashLight;
}

void APrototypeCharacter::ToggleEquip(const FInputActionValue& Value)
{
	if (!CombatComponent || !CombatComponent->GetEquippedWeapon()) return;

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!AnimInst) return;

	if (GetIsAiming())
	{
		StopAiming(Value);
	}

	bool bWillDraw = !CombatComponent->IsWeaponDrawn();
	// 캐릭터 헤더에 선언된 EquipMontage / UnEquipMontage를 사용
	UAnimMontage* MontageToPlay = bWillDraw ? EquipMontage : UnEquipMontage;

	// 무기의 데이터 에셋에서 몽타주 가져오기
	UWeaponData* CurrentWeaponData = CombatComponent->GetEquippedWeapon()->WeaponData;
	if (!CurrentWeaponData) return;

	if (MontageToPlay)
	{
		CombatComponent->ToggleEquip(MontageToPlay, AnimInst);
	}

	// 레이어 변경
	TSubclassOf<UAnimInstance> LayerToLink = bWillDraw ?
		((CombatComponent->GetCurrentWeaponIndex() == 1) ? PistolLayerClass : SMGLayerClass) :
		UnarmedLayerClass;

	AnimInst->LinkAnimClassLayers(LayerToLink);
}

void APrototypeCharacter::StartAiming(const FInputActionValue& Value)
{
	if (CombatComponent && CombatComponent->StartAiming())
	{
		//CameraBoom->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("clavicle_r"));

		//CameraBoom->SetRelativeLocation(FVector::ZeroVector);
		//CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);

		CameraBoom->bUsePawnControlRotation = true;
		CameraBoom->bInheritPitch = false;
		CameraBoom->bInheritYaw = true;
		CameraBoom->bInheritRoll = false;

		MainCamera->bUsePawnControlRotation = true;

		CameraBoom->bEnableCameraLag = false;
		CameraBoom->bEnableCameraRotationLag = false;

		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 0.5f;

		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			if (PC->PlayerCameraManager)
			{
				PC->PlayerCameraManager->ViewPitchMin = -20.0f;
				PC->PlayerCameraManager->ViewPitchMax = 10.0f;
			}

			if (AWZ_HUD_DH* HUD = Cast<AWZ_HUD_DH>(PC->GetHUD()))
			{
				HUD->SetCrosshairVisibility(true);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("조준 실패"));
	}
}

void APrototypeCharacter::StopAiming(const FInputActionValue& Value)
{
	if (CombatComponent) CombatComponent->StopAiming();

	bUseControllerRotationYaw = false;

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->ViewPitchMin = -60.0f;
			PC->PlayerCameraManager->ViewPitchMax = 50.0f;
		}

		if (AWZ_HUD_DH* HUD = Cast<AWZ_HUD_DH>(PC->GetHUD()))
		{
			HUD->SetCrosshairVisibility(false);
		}
	}

	CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bInheritPitch = true;
    CameraBoom->bInheritYaw = true;
    CameraBoom->bInheritRoll = false;
    
    if (MainCamera)
    {
        MainCamera->bUsePawnControlRotation = false;
        MainCamera->SetRelativeRotation(FRotator::ZeroRotator);
    }

    CameraBoom->bEnableCameraLag = true;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraLagSpeed = 15.0f;
    CameraBoom->CameraRotationLagSpeed = 15.0f;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		float SafeSpeed = (WalkSpeed > 0.0f) ? WalkSpeed : 600.0f;
		MoveComp->MaxWalkSpeed = SafeSpeed;
	}
}

void APrototypeCharacter::Fire(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->StartFire(FireMontage, GetMesh()->GetAnimInstance(), FireCameraShake);
	}
}

void APrototypeCharacter::StopFire(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->StopFire();
	}
}

bool APrototypeCharacter::GetIsRunning() const
{
	return bIsRunning;
}

bool APrototypeCharacter::GetIsPistolEquipped() const
{
	return CombatComponent ? CombatComponent->IsPistolEquipped() : false;
}

bool APrototypeCharacter::GetIsCrouching() const
{
	// ACharacter의 기본 내장 변수인 bIsCrouched 반환
	return bIsCrouched;
}

bool APrototypeCharacter::GetIsGround() const
{
	return GetCharacterMovement()->IsMovingOnGround();
}

bool APrototypeCharacter::GetIsQuickTurning() const
{
	return bIsQuickTurning;
}

int32 APrototypeCharacter::GetTurnIndex() const
{
	return TurnIndex;
}

bool APrototypeCharacter::IsEquipping() const
{
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		return AnimInst->Montage_IsPlaying(EquipMontage) || AnimInst->Montage_IsPlaying(UnEquipMontage);
	}
	return false;
}

bool APrototypeCharacter::GetIsAiming() const
{
	return CombatComponent ? CombatComponent->IsAiming() : false;
}

FVector APrototypeCharacter::GetHandIKTargetLoc() const
{
	return CombatComponent ? CombatComponent->GetHandIKTarget() : FVector::ZeroVector;
}

void APrototypeCharacter::SetIsQuickTurning(bool bIsTurning)
{
	bIsQuickTurning = bIsTurning;
	if (!bIsTurning) StopQuickTurn();
}

bool APrototypeCharacter::GetIsClimbing() const
{
	return bIsClimbing;
}

USkeletalMeshComponent* APrototypeCharacter::GetEquippedWeaponMesh()
{
	if (CombatComponent == nullptr) return nullptr;

	if (CombatComponent->EquippedWeapon == nullptr) return nullptr;

	return CombatComponent->EquippedWeapon->WeaponMesh;
}

AWeapon* APrototypeCharacter::GetEquippedWeapon()
{
	return CombatComponent ? CombatComponent->GetEquippedWeapon() : nullptr;
}

bool APrototypeCharacter::GetIsReloading() const
{
	return CombatComponent ? CombatComponent->GetIsReloading() : false;
}

bool APrototypeCharacter::GetIsSMGEquipped() const
{
	if (CombatComponent)
	{
		return (CombatComponent->CurrentWeaponIndex == 2) && CombatComponent->IsWeaponDrawn();
	}
	return false;
}

int32 APrototypeCharacter::GetCurrentWeaponIndex() const
{
	return CombatComponent ? CombatComponent->GetCurrentWeaponIndex() : 0;
}

float APrototypeCharacter::GetAimPitch() const
{
	return CombatComponent ? CombatComponent->GetAimPitch() : 0.0f;
}

float APrototypeCharacter::GetAimYaw() const
{
	return CombatComponent ? CombatComponent->GetAimYaw() : 0.0f;
}

bool APrototypeCharacter::IsFiring() const
{
	return CombatComponent ? CombatComponent->IsFiring() : false;
}

void APrototypeCharacter::PlayFootstepSound(FName FootBoneName)
{
	FVector FootLocation = GetMesh()->GetSocketLocation(FootBoneName);

	FVector Start = FootLocation;
	FVector End = Start - FVector(0.0f, 0.0f, 50.0f);
	FHitResult HitResult;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); 

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		USoundBase* SoundToPlay = Sound_DefaultStep; // 기본 소리

		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			if (HitActor->ActorHasTag(TEXT("Metal")))
			{
				SoundToPlay = Sound_MetalStep;
			}
		}

		// 최종 결정된 소리를 재생
		if (SoundToPlay)
		{
			UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, HitResult.ImpactPoint);
		}
	}

}

void APrototypeCharacter::SelectWeapon1(const FInputActionValue& Value)
{
	if (!CombatComponent) return;
	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!AnimInst) return;

	if (GetIsAiming())
	{
		StopAiming(Value);
	}

	if (CombatComponent->GetCurrentWeaponIndex() == 1 && CombatComponent->IsWeaponDrawn())
	{
		ToggleEquip(Value);
		return;
	}

	CombatComponent->ChangeWeapon(1, AnimInst);

	UAnimMontage* EquipAnim = CombatComponent->GetCurrentEquipMontage(true);
	if (EquipAnim)
	{
		AnimInst->Montage_Play(EquipAnim);
		CombatComponent->SetIsWeaponDrawn(true);
	}

	AnimInst->LinkAnimClassLayers(PistolLayerClass);
}

void APrototypeCharacter::SelectWeapon2(const FInputActionValue& Value)
{
	if (!CombatComponent) return;
	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!AnimInst) return;

	if (GetIsAiming())
	{
		StopAiming(Value);
	}

	if (bIsUseFlashLight)
	{
		bIsUseFlashLight = false;
		ToggleLight(false);
	}

	if (CombatComponent->GetCurrentWeaponIndex() == 2 && CombatComponent->IsWeaponDrawn())
	{
		ToggleEquip(Value);
		return;
	}

	// 무기 변경 (이제 내부에서 SMG를 보이게 놔둡니다)
	CombatComponent->ChangeWeapon(2, AnimInst);

	// SMG를 꺼내는 몽타주 재생
	UAnimMontage* EquipAnim = CombatComponent->GetCurrentEquipMontage(true);
	if (EquipAnim)
	{
		AnimInst->Montage_Play(EquipAnim);
		// 상태 동기화
		CombatComponent->SetIsWeaponDrawn(true);
	}

	AnimInst->LinkAnimClassLayers(SMGLayerClass);
}