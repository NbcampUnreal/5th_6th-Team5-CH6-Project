#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/PlayerStatusComponent.h"
#include "Character/Components/PlayerCombatComponent.h"
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

APrototypeCharacter::APrototypeCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 컴포넌트 생성
	StatusComponent = CreateDefaultSubobject<UPlayerStatusComponent>(TEXT("StatusComp"));
	CombatComponent = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("CombatComp"));

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

	if (CameraBoom)
	{
		StandingArmLength = CameraBoom->TargetArmLength;
		OriginalTargetOffset = CameraBoom->TargetOffset;
		OriginalSocketOffset = CameraBoom->SocketOffset;
		OriginalFOV = MainCamera->FieldOfView;
	}

	if (CombatComponent)
	{
		CombatComponent->SetupCombat(MainCamera); // 여기서 카메라를 넘겨줘야 함!
	}

	if (StatusComponent)
	{
		StatusComponent->OnPlayerDied.AddDynamic(this, &APrototypeCharacter::OnDeath);
	}
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		if (UnarmedLayerClass)
		{
			AnimInst->LinkAnimClassLayers(UnarmedLayerClass);
		}
	}
}

void APrototypeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckRunState();
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

	if (CombatComponent && CombatComponent->IsAiming())
	{
		FRotator NewRot = GetActorRotation();
		NewRot.Yaw = GetControlRotation().Yaw;
		SetActorRotation(NewRot);		

	}

	// 이하 평상시 회전 및 카메라 로직만 실행
	if (!bIsRunning && !bIsQuickTurning && GetVelocity().SizeSquared() > KINDA_SMALL_NUMBER)

	if (bIsRunning)
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;

		bUseControllerRotationYaw = false;

		float CurrentSpeed = GetVelocity().SizeSquared();
		if (CurrentSpeed > 10.0f) // 움직일 때만
		{
			FRotator TargetRotation = FRotator(0.0f, GetControlRotation().Yaw, 0.0f);
			FRotator CurrentRotation = GetActorRotation();
			FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f);
			SetActorRotation(NewRotation);
		}
	}

#pragma region Camera Aim & Bobbing

	float TargetArmLengthDest;
	float TargetFOVDest;
	FVector TargetSocketOffsetDest;
	FVector TargetTargetOffsetDest;

	if (CombatComponent && CombatComponent->IsAiming())
	{
		// [조준 시 목표]
		TargetArmLengthDest = AimArmLength;
		TargetFOVDest = AimFOV;          

		TargetSocketOffsetDest = AimSocketOffset;

		TargetTargetOffsetDest = FVector::ZeroVector;

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
		TargetSocketOffsetDest.Y += YOffsetBob;
		TargetSocketOffsetDest.Z += ZOffsetBob;
		
	}

	float InterpSpeed = AimInterpSpeed;

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLengthDest, DeltaTime, InterpSpeed);
	MainCamera->FieldOfView = FMath::FInterpTo(MainCamera->FieldOfView, TargetFOVDest, DeltaTime, InterpSpeed);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffsetDest, DeltaTime, InterpSpeed);
	CameraBoom->TargetOffset = FMath::VInterpTo(CameraBoom->TargetOffset, TargetTargetOffsetDest, DeltaTime, InterpSpeed);

#pragma endregion

	// 등반 처리
	if (bIsClimbing && CurrentLadder)
	{
		float MyHeight = GetActorLocation().Z;
		float TopHeight = CurrentLadder->TopExitPoint->GetComponentLocation().Z;
		float BottomHeight = CurrentLadder->BottomStartPoint->GetComponentLocation().Z;

		if (MyHeight >= TopHeight)
		{
			StopClimbing();
			SetActorLocation(CurrentLadder->TopExitPoint->GetComponentLocation());
		}
		else if (MyHeight <= BottomHeight)
		{
			StopClimbing();
		}
		return;
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
	if (CombatComponent && CombatComponent->IsAiming())
	{
		CombatComponent->StopAiming();
	}

	EPlayerHitDirection HitDir = GetHitDirection(ToAttackerDir);
	UAnimMontage* MontageToPlay = nullptr;

	switch (HitDir)
	{
	case EPlayerHitDirection::Front: MontageToPlay = HitMontage_Front; break;
	case EPlayerHitDirection::Back: MontageToPlay = HitMontage_Back; break;
	case EPlayerHitDirection::Right: MontageToPlay = HitMontage_Right ? HitMontage_Right : HitMontage_Front; break;
	case EPlayerHitDirection::Left: MontageToPlay = HitMontage_Left ? HitMontage_Left : HitMontage_Front; break;
	}

	if (MontageToPlay)
	{
		PlayAnimMontage(MontageToPlay);
	}
}

void APrototypeCharacter::PlayDeathReaction(const FVector& ToAttackerDir)
{
	EPlayerHitDirection HitDir = GetHitDirection(ToAttackerDir);
	UAnimMontage* MontageToPlay = nullptr;

	switch (HitDir)
	{
	case EPlayerHitDirection::Front: MontageToPlay = DeathMontage_Front; break;
	case EPlayerHitDirection::Back: MontageToPlay = DeathMontage_Back; break;
	case EPlayerHitDirection::Right: MontageToPlay = DeathMontage_Right; break;
	case EPlayerHitDirection::Left: MontageToPlay = DeathMontage_Left; break;
	}

	if (MontageToPlay)
	{
		PlayAnimMontage(MontageToPlay);
	}
	else
	{
		GetMesh()->SetSimulatePhysics(true);
	}
}

void APrototypeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction) EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Move);
		if (LookAction) EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Look);
		if (RunAction)
		{
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &APrototypeCharacter::StartRunning);
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &APrototypeCharacter::EndRunning);
		}
		if (CrouchAction) EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &APrototypeCharacter::ToggleCrouch);
		if (InteractAction) EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APrototypeCharacter::Interact);

		if (EquipAction) EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &APrototypeCharacter::ToggleEquip);
		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APrototypeCharacter::StartAiming);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APrototypeCharacter::StopAiming);
		}
		if (FireAction) EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APrototypeCharacter::Fire);

		if (ReloadAction)
		{
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &APrototypeCharacter::Reload);
		}

		if (QuickTurnAction)
		{
			EnhancedInputComponent->BindAction(QuickTurnAction, ETriggerEvent::Started, this, &APrototypeCharacter::PerformQuickTurn180);
		}
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

	if (bIsCrouched || bIsRunning || bIsClimbing) return;

	bIsRunning = true;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	CameraBoom->CameraLagSpeed = 10.0f;
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
		float CurrentSpeed = GetVelocity().Size();
		if (CurrentSpeed <= KINDA_SMALL_NUMBER) // 멈춤
		{
			bIsRunning = false;

			if (CombatComponent && CombatComponent->IsAiming())
			{
				GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 0.5f;
			}
			else
			{
				GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			}		

			CameraBoom->CameraLagSpeed = 15.0f;
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

void APrototypeCharacter::Interact(const FInputActionValue& Value)
{
	if (bIsClimbing)
	{
		StopClimbing();
		return;
	}

	FVector Start = MainCamera->GetComponentLocation();
	FVector End = Start + (MainCamera->GetForwardVector() * 250.0f);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteract::StaticClass()))
		{
			IInteract::Execute_OnInteract(HitActor, this);
		}
	}
}

void APrototypeCharacter::StartClimbing(ALadder* Ladder)
{
	if (!Ladder) return;

	bIsClimbing = true;
	CurrentLadder = Ladder;
	bIsRunning = false;
	UnCrouch();

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->MaxWalkSpeed = ClimbSpeed;

	FVector StartLoc = Ladder->BottomStartPoint->GetComponentLocation();
	FRotator StartRot = Ladder->GetActorForwardVector().Rotation();
	StartRot.Yaw += 180.0f;

	SetActorLocationAndRotation(StartLoc, StartRot);
}

void APrototypeCharacter::StopClimbing()
{
	bIsClimbing = false;
	CurrentLadder = nullptr;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APrototypeCharacter::ToggleEquip(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		UAnimMontage* MontageToPlay = CombatComponent->bIsPistolEquipped ? UnEquipMontage : EquipMontage;

		CombatComponent->ToggleEquip(MontageToPlay, GetMesh()->GetAnimInstance());

		TSubclassOf<class UAnimInstance> LayerToLink = CombatComponent->IsPistolEquipped() ? PistolLayerClass : UnarmedLayerClass;

		if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
		{
			if (LayerToLink)
			{
				AnimInst->LinkAnimClassLayers(LayerToLink);
			}
		}
	}
}

void APrototypeCharacter::StartAiming(const FInputActionValue& Value)
{
	if (CombatComponent && CombatComponent->StartAiming())
	{
		CameraBoom->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("clavicle_r"));

		CameraBoom->SetRelativeLocation(FVector::ZeroVector);
		CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);

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
			if (AWZ_HUD_DH* HUD = Cast<AWZ_HUD_DH>(PC->GetHUD()))
			{
				HUD->SetCrosshairVisibility(true); // 켜!
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
	if (CombatComponent)
	{
		CombatComponent->StopAiming();
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (AWZ_HUD_DH* HUD = Cast<AWZ_HUD_DH>(PC->GetHUD()))
		{
			HUD->SetCrosshairVisibility(false); // 꺼!
		}
	}

	if (CameraBoom)
	{
		CameraBoom->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		CameraBoom->SetWorldLocationAndRotation(GetActorLocation(), GetActorRotation());
		CameraBoom->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("root"));

		// 보스 코드 원본 복구
		CameraBoom->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		// 위치 미세 조정
		CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
		CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);

		// 설정 복구
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
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void APrototypeCharacter::Fire(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->Fire(FireMontage, GetMesh()->GetAnimInstance(), FireCameraShake);
		CombatComponent->HandIKTargetLocation += FVector(0.0f, 0.0f, 20.0f);
	}
}