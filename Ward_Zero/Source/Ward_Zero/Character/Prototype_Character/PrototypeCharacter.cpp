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
	CameraBoom->TargetArmLength = StandingArmLength;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 15.0f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 15.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 45.0f, 30.0f);
	CameraBoom->ProbeSize = 12.0f;

	// 카메라 생성 및 설정
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	MainCamera->bUsePawnControlRotation = false;
	MainCamera->FieldOfView = 60.0f;

	// 권총 메쉬
	PistolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PistolMesh"));
	if (PistolMesh)
	{
		PistolMesh->SetupAttachment(GetMesh(), TEXT("WeaponSocket"));
		PistolMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PistolMesh->SetVisibility(false);
		PistolMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

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

	StandingArmLength = CameraBoom->TargetArmLength;
	DefaultTargetOffset = CameraBoom->TargetOffset;

	// 컴포넌트 초기화 및 델리게이트 연결
	if (CombatComponent)
	{
		CombatComponent->SetupCombat(PistolMesh, MainCamera);
	}

	if (StatusComponent)
	{
		StatusComponent->OnPlayerDied.AddDynamic(this, &APrototypeCharacter::OnDeath);
	}
}

void APrototypeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckRunState();
	if (bIsQuickTurning) return;

	// 카메라 줌 로직 
	float TargetArmLength = StandingArmLength;
	float TargetFOV = 90.0f;

	// bIsAiming 변수 대신 컴포넌트에게 물어봄
	if (CombatComponent && CombatComponent->IsAiming())
	{
		TargetArmLength = AimArmLength;
		TargetFOV = AimFOV;

		// 조준 중 회전 동기화
		FRotator NewRot = GetActorRotation();
		NewRot.Yaw = GetControlRotation().Yaw;
		SetActorRotation(NewRot);
	}
	else
	{
		// 기존 이동 회전 로직 유지
		if (!bIsRunning && GetVelocity().SizeSquared() > KINDA_SMALL_NUMBER)
		{
			FRotator TargetRotation = FRotator(0.0f, GetControlRotation().Yaw, 0.0f);
			FRotator CurrentRotation = GetActorRotation();
			FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, WalkTurnRate);
			SetActorRotation(NewRotation);
		}
	}

	// 카메라 보간
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, 15.0f);
	MainCamera->FieldOfView = FMath::FInterpTo(MainCamera->FieldOfView, TargetFOV, DeltaTime, 15.0f);
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

	// 필요 시 사망 몽타주 재생 추가할 예정 
}

EPlayerHitDirection APrototypeCharacter::GetHitDirection(const FVector& ToAttackerDir)
{
	FVector ToAttacker = (ToAttackerDir - GetActorLocation()).GetSafeNormal();
	FVector MyForward = GetActorForwardVector();
	FVector MyRight = GetActorRightVector();

	float ForwardDot = FVector::DotProduct(MyForward, ToAttacker);
	float RightDot = FVector::DotProduct(MyRight, ToAttacker);

	if (ForwardDot >= 0.5f) return EPlayerHitDirection::Front;
	else if (ForwardDot <= -0.5f) return EPlayerHitDirection::Back;
	else return (RightDot > 0.f) ? EPlayerHitDirection::Right : EPlayerHitDirection::Left;
}

void APrototypeCharacter::PlayHitReaction(const FVector& ToAttackerDir)
{
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
	// (기존 바인딩 코드 유지)
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction) EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Move);
		if (LookAction) EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Look);
		if (RunAction) EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &APrototypeCharacter::StartRunning);
		if (CrouchAction) EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &APrototypeCharacter::ToggleCrouch);
		if (InteractAction) EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APrototypeCharacter::Interact);

		if (EquipAction) EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &APrototypeCharacter::ToggleEquip);
		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APrototypeCharacter::StartAiming);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APrototypeCharacter::StopAiming);
		}
		if (FireAction) EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APrototypeCharacter::Fire);
	}
}

void APrototypeCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (bIsClimbing)
	{
		AddMovementInput(FVector::UpVector, MovementVector.Y);
		return;
	}
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
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
	if (bIsCrouched || bIsRunning || bIsClimbing) return;

	bIsRunning = true;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	CameraBoom->CameraLagSpeed = 10.0f;
}

void APrototypeCharacter::CheckRunState()
{
	if (bIsRunning)
	{
		float CurrentSpeed = GetVelocity().Size();
		if (CurrentSpeed <= KINDA_SMALL_NUMBER)
		{
			bIsRunning = false;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			GetCharacterMovement()->bOrientRotationToMovement = false;
			CameraBoom->CameraLagSpeed = 15.0f;
		}
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
		CombatComponent->ToggleEquip(EquipMontage, GetMesh()->GetAnimInstance());
	}
}

void APrototypeCharacter::StartAiming(const FInputActionValue& Value)
{
	if (bIsRunning || bIsClimbing) return; // 달리기, 등반 중 조준 불가

	if (CombatComponent)
	{
		CombatComponent->StartAiming();
	}
}

void APrototypeCharacter::StopAiming(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->StopAiming();
	}
}

void APrototypeCharacter::Fire(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->Fire(FireMontage, GetMesh()->GetAnimInstance(), FireCameraShake);
	}
}