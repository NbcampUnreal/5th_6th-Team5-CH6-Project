#include "Prototype_Character/PrototypeCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI_KWJ/Reading/DocumentSubsystem.h"
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

	// 캐릭터 회전 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 캐릭터 무브먼트 설정
	GetCharacterMovement()->bOrientRotationToMovement = false; // 이동 방향으로 캐릭터 회전
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 240.0f, 0.0f); // 회전 속도
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; // 걷기 속도
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.f; // 멈출 때의 감속도
	GetCharacterMovement()->MaxAcceleration = 800.f;
	GetCharacterMovement()->GroundFriction = 6.f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true; // 웅크리기 가능
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchMovementSpeed;

	// 카메라 붐 생성 및 설정
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = StandingArmLength; // 캐릭터와의 거리
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true; // 무거운 조작감
	CameraBoom->CameraLagSpeed = 15.0f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 15.0f;

	// 카메라 붐 오프셋 설정
	CameraBoom->SocketOffset = FVector(0.0f, 45.0f, 30.0f);

	// 카메라 생성 및 설정
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	MainCamera->bUsePawnControlRotation = false;
	MainCamera->FieldOfView = 60.0f;

	PistolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PistolMesh"));
	if (PistolMesh)
	{
		PistolMesh->SetupAttachment(GetMesh(), TEXT("WeaponSocket"));
		PistolMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PistolMesh->SetVisibility(false);
	}

	LaserSightComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LaserSight"));
	LaserSightComponent->SetupAttachment(PistolMesh, TEXT("Muzzle"));
	LaserSightComponent->bAutoActivate = false; 

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
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->ViewPitchMin = -60.0f;
			PlayerController->PlayerCameraManager->ViewPitchMax = 50.0f;
		}
	}

	StandingArmLength = CameraBoom->TargetArmLength;
	DefaultTargetOffset = CameraBoom->TargetOffset;

	CurrHealth = MaxHealth;
	bIsDead = false;
}

void APrototypeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckRunState();
	if (bIsQuickTurning) return;

	//회전 동기화 
	if (bIsAiming)
	{
		// 컨트롤러(카메라)가 보는 방향으로 캐릭터 몸통을 즉시 돌림
		FRotator NewRot = GetActorRotation();
		NewRot.Yaw = GetControlRotation().Yaw;
		SetActorRotation(NewRot);
		// Aim Offset 계산
		CalculateAimOffset();
		UpdateLaserSight();
		return;
	}
	else
	{
		if (!bIsRunning && GetVelocity().SizeSquared() > KINDA_SMALL_NUMBER)
		{
			FRotator TargetRotation = FRotator(0.0f, GetControlRotation().Yaw, 0.0f);
			FRotator CurrentRotation = GetActorRotation();
			FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, WalkTurnRate);
			SetActorRotation(NewRotation);
		}
	}

	float TargetBaseZ = bIsCrouched ? CrouchedCameraHeight : StandingCameraHeight;
	float TargetArmLength = bIsCrouched ? CrouchedArmLength : StandingArmLength;

	CurrentBaseCameraZ = FMath::FInterpTo(CurrentBaseCameraZ, TargetBaseZ, DeltaTime, 5.0f);
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, 5.0f);

	FVector Velocity = GetVelocity();
	float Speed = Velocity.Size();

	float ZOffsetBob = 0.0f;
	float YOffsetBob = 0.0f;

	if (Speed > KINDA_SMALL_NUMBER && GetCharacterMovement()->IsMovingOnGround())
	{
		BobTime += DeltaTime * (Speed / 150.0f) * BobFrequency;
		ZOffsetBob = FMath::Sin(BobTime) * BobAmplitude;
		YOffsetBob = FMath::Cos(BobTime * 0.5f) * BobHorizontalAmplitude;
	}
	else
	{
		ZOffsetBob = 0.0f;
		YOffsetBob = 0.0f;

		BobTime = 0.0f;
	}
	float ActualTargetArmLength = bIsAiming ? AimArmLength : StandingArmLength;
	float ActualTargetFOV = bIsAiming ? AimFOV : 80.f;

	// TargetOffset: 회전의 중심축
	FVector AimTargetOffsetVal = FVector(0.0f, 65.0f, 50.0f);

	// SocketOffset: 렌즈 위치
	FVector AimSocketOffsetVal = FVector(0.0f, 0.0f, -15.0f);

	// 2. 카메라 위치 목표값 설정
	float TargetArmLengthDest;
	float TargetFOVDest;
	FVector TargetSocketOffsetDest;
	FVector TargetOffsetDest;

	if (bIsAiming)
	{
		// 캐릭터 상체 + 총 든 손을 좌측 하단에 박제
		TargetArmLengthDest = 40.0f; 
		TargetFOVDest = 45.0f;

		TargetOffsetDest = FVector(0.0f, 70.0f, 65.0f);
		TargetSocketOffsetDest = FVector(-20.0f, 0.0f, -25.0f);
	}
	else
	{
		// [평상시] 유저님이 만족하셨던 기존 위치 유지 (보정)
		TargetArmLengthDest = StandingArmLength;
		TargetFOVDest = 80.0f;

		float BobZ = StandingCameraHeight + ZOffsetBob;
		float BobY = 40.0f + YOffsetBob;

		TargetOffsetDest = FVector(0.0f, 0.0f, 45.0f);
		TargetSocketOffsetDest = FVector(0.0f, BobY, BobZ);
	}

	// 보간 속도 설정
	float InterpSpeed = bIsAiming ? 40.0f : 12.0f;

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLengthDest, DeltaTime, InterpSpeed);
	MainCamera->FieldOfView = FMath::FInterpTo(MainCamera->FieldOfView, TargetFOVDest, DeltaTime, InterpSpeed);

	CameraBoom->TargetOffset = FMath::VInterpTo(CameraBoom->TargetOffset, TargetOffsetDest, DeltaTime, InterpSpeed);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffsetDest, DeltaTime, InterpSpeed);
	//추가 - JC End 
	
	//Climb
	if (bIsClimbing && CurrentLadder)
	{
		float MyHeight = GetActorLocation().Z;
		float TopHeight = CurrentLadder->TopExitPoint->GetComponentLocation().Z;
		float BottomHeight = CurrentLadder->BottomStartPoint->GetComponentLocation().Z;

		if (MyHeight >= TopHeight) //사다리 끝 도달 
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

	if (bIsAiming)
	{
		UpdateLaserSight();
	}
}

void APrototypeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 이동
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Move);
		}

		// 시점
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Look);
		}

		// 달리기
		if (RunAction)
		{
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &APrototypeCharacter::StartRunning);
		}

		// 앉기
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &APrototypeCharacter::ToggleCrouch);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APrototypeCharacter::Interact);
		}

		if (EquipAction)
		{
			EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &APrototypeCharacter::ToggleEquip);
		}

		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APrototypeCharacter::StartAiming);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APrototypeCharacter::StopAiming);
		}

		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APrototypeCharacter::Fire);
		}
	}

}

float APrototypeCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.f;

	// 부모 클래스에서 데미지 처리 
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	ActualDamage = FMath::Min(CurrHealth, ActualDamage);
	CurrHealth -= ActualDamage;

	UE_LOG(LogTemp, Warning, TEXT("Player Hit! Current HP: %.f"), CurrHealth);

	//공격이 날아온 방향
	 
	//총에 맞은 경우 == 총알의 방향 정보 
	FVector ToAttackerDir = FVector::ZeroVector;

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);

		ToAttackerDir = -PointEvent->ShotDirection;
		ToAttackerDir.Normalize();
	}
	//근접 공격 
	else if (DamageCauser)
	{
		ToAttackerDir = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	}

	if (CurrHealth <= 0.f)
	{
		// 사망 
		bIsDead = true; 
		PlayDeathReaction(ToAttackerDir);

		// 입력 제한 
		DisableInput(Cast<APlayerController>(Controller));
		
		// 캡슐 콜리젼 끄기 
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	}
	else
	{
		PlayHitReaction(ToAttackerDir);
	}

	return ActualDamage;
}

EPlayerHitDirection APrototypeCharacter::GetHitDirection(const FVector& ToAttackerDir)
{
	// 공격한 AI가 공격한 방향을 가지고 몽타주 분류 
	FVector ToAttacker = (ToAttackerDir - GetActorLocation()).GetSafeNormal();
	FVector MyForward = GetActorForwardVector();
	FVector MyRight = GetActorRightVector();

	float ForwardDot = FVector::DotProduct(MyForward, ToAttacker);
	float RightDot = FVector::DotProduct(MyRight, ToAttacker);

	if (ForwardDot >= 0.5f)
	{
		//AI == 내 앞 
		return EPlayerHitDirection::Front;
	}
	else if (ForwardDot <= -0.5f)
	{
		//AI = 내 뒤 
		return EPlayerHitDirection::Back;
	}
	else
	{
		if (RightDot > 0.f)
		{
			// 양수 = 오른쪽 
			return EPlayerHitDirection::Right;
		}
		else
		{
			// 음수 = 왼쪽 
			return EPlayerHitDirection::Left;
		}
	}
}

void APrototypeCharacter::PlayHitReaction(const FVector& ToAttackerDir)
{
	//맞는 순간 조준 풀기 
	if (bIsAiming)
	{
		StopAiming(FInputActionValue());
	}

	EPlayerHitDirection HitDir = GetHitDirection(ToAttackerDir);
	UAnimMontage* MontageToPlay = nullptr; 
	
	switch (HitDir)
	{
	case EPlayerHitDirection::Front:
		MontageToPlay = HitMontage_Front;
		break;
	case EPlayerHitDirection::Back:
		MontageToPlay = HitMontage_Back;
		break;
	case EPlayerHitDirection::Right:
		MontageToPlay = HitMontage_Right ? HitMontage_Right : HitMontage_Front;
		break;
	case EPlayerHitDirection::Left:
		MontageToPlay = HitMontage_Left ? HitMontage_Left : HitMontage_Front;
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
	case EPlayerHitDirection::Front:
		MontageToPlay = DeathMontage_Front;
		break;
	case EPlayerHitDirection::Back:
		MontageToPlay = DeathMontage_Back;
		break;
	case EPlayerHitDirection::Right:
		MontageToPlay = DeathMontage_Right;
		break;
	case EPlayerHitDirection::Left:
		MontageToPlay = DeathMontage_Left;
		break;
	}

	if (MontageToPlay)
	{
		PlayAnimMontage(MontageToPlay);
	}
	else
	{
		//없으면 래그돌 
		GetMesh()->SetSimulatePhysics(true);
	}
}

void APrototypeCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (bIsClimbing) //등반 중 
	{
		// W/S로 이동 
		AddMovementInput(FVector::UpVector, MovementVector.Y);
		return;
	}
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// 전방 벡터
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// 우측 벡터
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// 이동 입력 적용
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APrototypeCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	UE_LOG(LogTemp, Warning, TEXT("Look Input: %f, %f"), LookAxisVector.X, LookAxisVector.Y);

	if (Controller != nullptr)
	{
		// 조준 중이라면 감도를 낮춤
		float CurrentSensitivity = bIsAiming ? AimLookSensitivity : 1.0f;
		//변경 사항 = 조준 시 감도 감소 
		AddControllerYawInput(LookAxisVector.X * CurrentSensitivity);
		AddControllerPitchInput(LookAxisVector.Y * CurrentSensitivity);
	}
}

void APrototypeCharacter::ToggleCrouch(const FInputActionValue& Value)
{
	// 달리고 있다면 앉기 불가능
	if (bIsRunning) return;
	//등반 중이면 앉기 불가능
	if (bIsClimbing) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void APrototypeCharacter::StartRunning(const FInputActionValue& Value)
{
	if (bIsCrouched)
	{
		return;
	}

	if (bIsRunning)
	{
		return;
	}

	if (bIsClimbing)
	{
		return;
	}

	bIsRunning = true;

	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;

	// 회전 모드로 변경
	GetCharacterMovement()->bOrientRotationToMovement = true;
	CameraBoom->CameraLagSpeed = 10.0f;
}

void APrototypeCharacter::CheckRunState()
{
	if (bIsRunning)
	{
		float CurrentSpeed = GetVelocity().Size();

		if(CurrentSpeed <= KINDA_SMALL_NUMBER)
		{
			bIsRunning = false;

			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

			// 회전 모드 복구
			GetCharacterMovement()->bOrientRotationToMovement = false;

			CameraBoom->CameraLagSpeed = 15.0f;
		}
	}
}

void APrototypeCharacter::Interact(const FInputActionValue& Value)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		ULocalPlayer* LP = PC->GetLocalPlayer();
		if (LP)
		{
			UDocumentSubsystem* DocSubsystem = LP->GetSubsystem<UDocumentSubsystem>();
			if (DocSubsystem && DocSubsystem->IsDocumentOpen())
			{
				DocSubsystem->CloseDocument();
				return;
			}
		}
	}
	if (bIsClimbing)
	{
		StopClimbing(); 
		return;
	}

	//전방 사다리 있는지 검사 
	FVector Start = MainCamera->GetComponentLocation();
	FVector End = Start + (MainCamera->GetForwardVector() * 250.0f);
	
	FHitResult Hit; 
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = Hit.GetActor();
		//히트된 엑터 = 인터페이스 구현 여부 검사 
		if (HitActor && HitActor->GetClass()->ImplementsInterface(UInteract::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Interact Hit"));
			IInteract::Execute_OnInteract(HitActor, this);
		}
	}
}

void APrototypeCharacter::ToggleEquip(const FInputActionValue& Value)
{
	if (!IsValid(PistolMesh)) return;
	if (bIsAiming) return;

	bIsPistolEquipped = !bIsPistolEquipped;

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!AnimInst || !EquipMontage) return;

	if (!PistolMesh->IsVisible())
	{
		AnimInst->Montage_Play(EquipMontage);
	}
	PistolMesh->SetVisibility(bIsPistolEquipped);
	//총 내리는 몽타주 추가 예정 
}

void APrototypeCharacter::StartAiming(const FInputActionValue& Value)
{
	if (bIsPistolEquipped && !bIsRunning && !bIsClimbing)
	{
		bIsAiming = true;
		bIsAiming_Anim = true;

		bWasUsingPawnControlRotation = CameraBoom->bUsePawnControlRotation;
		OriginalSocketOffset = CameraBoom->SocketOffset;
		OriginalTargetOffset = CameraBoom->TargetOffset;

		CameraBoom->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("clavicle_r")
		);

		CameraBoom->SetRelativeLocation(FVector(-25.0f, -10.0f, -20.f));
		CameraBoom->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

		CameraBoom->TargetArmLength = 80.0f;

		CameraBoom->bUsePawnControlRotation = true;
		CameraBoom->bInheritPitch = true;
		CameraBoom->bInheritYaw = true;
		CameraBoom->bInheritRoll = false;

		CameraBoom->SocketOffset = FVector::ZeroVector;
		CameraBoom->TargetOffset = FVector::ZeroVector;

		MainCamera->FieldOfView = 50.0f;

		CameraBoom->bEnableCameraLag = false;
		CameraBoom->bEnableCameraRotationLag = false;

		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 0.5f;

		if (LaserSightComponent) LaserSightComponent->Activate(true);
	}
}

void APrototypeCharacter::StopAiming(const FInputActionValue& Value)
{
	bIsAiming = false;
	bIsAiming_Anim = false;

	// 카메라를 원래 위치로 복귀

	// SpringArm을 다시 RootComponent에 부착
	CameraBoom->AttachToComponent(
		RootComponent,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	// 원래 위치로 복구
	CameraBoom->SetRelativeLocation(FVector::ZeroVector);
	CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);

	CameraBoom->TargetArmLength = StandingArmLength;
	CameraBoom->SocketOffset = OriginalSocketOffset;
	CameraBoom->TargetOffset = OriginalTargetOffset;
	CameraBoom->bUsePawnControlRotation = bWasUsingPawnControlRotation;
	MainCamera->FieldOfView = 80.0f;

	// Lag 복구
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraLagSpeed = 15.0f;

	// 캐릭터 회전 복구
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	if (LaserSightComponent) LaserSightComponent->Deactivate();
}

void APrototypeCharacter::Fire(const FInputActionValue& Value)
{
	if (!bIsAiming) return;

	//총구 화염 
	if (MuzzleFlash)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlash, PistolMesh, TEXT("Muzzle"),
			FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}
	
	FVector Start = MainCamera->GetComponentLocation();
	FVector End = Start + (MainCamera->GetForwardVector() * 5000.f);

	FHitResult Hit;
	FCollisionQueryParams Params; 
	Params.AddIgnoredActor(this);

	FVector TargetPoint = End;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		TargetPoint = Hit.ImpactPoint;
		//피격 이펙트 
		if (ImpactEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),ImpactEffect,Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		}
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10.f, 12, FColor::Red, false, 2.0f);
		//데미지 처리 여기서 
	}

	HandIKTargetLocation += FVector(0.0f, 0.0f, 20.0f);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(FireMontage);
	} 

	//카메라 쉐이크 
	if (FireCameraShake)
	{
		GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(FireCameraShake);
	}
}

void APrototypeCharacter::StartClimbing(ALadder* Ladder)
{
	if (!Ladder) return;

	bIsClimbing = true;
	CurrentLadder = Ladder; 
	bIsRunning = false; 
	UnCrouch();

	GetCharacterMovement()->SetMovementMode(MOVE_Flying); //중력 무시 
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->MaxWalkSpeed = ClimbSpeed;

	//플레이어 위치를 사다리 중간으로 이동 
	FVector StartLoc = Ladder->BottomStartPoint->GetComponentLocation();
	FRotator StartRot = Ladder->GetActorForwardVector().Rotation(); // 방향 
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

void APrototypeCharacter::UpdateLaserSight()
{
	if (!bIsAiming || !PistolMesh || !LaserSightComponent) return;

	// 총구 위치
	FVector MuzzleLoc = PistolMesh->GetSocketLocation(TEXT("Muzzle"));

	// 카메라 중앙에서 레이캐스트
	FVector CameraLoc = MainCamera->GetComponentLocation();
	FVector CameraForward = MainCamera->GetForwardVector();
	FVector TraceEnd = CameraLoc + (CameraForward * 10000.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, CameraLoc, TraceEnd, ECC_Visibility, Params))
	{
		TraceEnd = Hit.ImpactPoint;
	}

	// IK 타겟 설정 (손이 이 지점을 향하도록)
	HandIKTargetLocation = TraceEnd;

	// 레이저는 총구에서 타겟까지
	LaserSightComponent->SetNiagaraVariableVec3(TEXT("BeamEnd"), TraceEnd);
}

void APrototypeCharacter::CalculateAimOffset()
{
	if (!bIsAiming) return;

	// 카메라가 보는 방향
	FVector CameraForward = MainCamera->GetForwardVector();

	// 캐릭터의 정면 방향 (Yaw만 사용)
	FRotator CharacterRotation = GetActorRotation();
	FVector CharacterForward = CharacterRotation.Vector();

	// 캐릭터 기준 상대 회전 계산
	FRotator DeltaRotation = (CameraForward.Rotation() - CharacterRotation);
	DeltaRotation.Normalize(); // -180 ~ 180 범위로 정규화

	// Aim Offset 값 설정
	AimYaw = DeltaRotation.Yaw;
	AimPitch = DeltaRotation.Pitch;

	// 범위 제한 (필요시)
	AimYaw = FMath::Clamp(AimYaw, -90.0f, 90.0f);
	AimPitch = FMath::Clamp(AimPitch, -90.0f, 90.0f);
}