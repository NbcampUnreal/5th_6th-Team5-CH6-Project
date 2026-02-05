#include "Prototype_Character/PrototypeCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

APrototypeCharacter::APrototypeCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 캐릭터 회전 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 캐릭터 무브먼트 설정
	GetCharacterMovement()->bOrientRotationToMovement = false; // 이동 방향으로 캐릭터 회전
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f); // 회전 속도
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; // 걷기 속도
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1500.f; // 멈출 때의 감속도
	GetCharacterMovement()->MaxAcceleration = 1000.f;
	GetCharacterMovement()->GroundFriction = 6.f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true; // 웅크리기 가능
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchMovementSpeed;

	// 카메라 붐 생성 및 설정
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 200.0f; // 캐릭터와의 거리
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
	MainCamera->FieldOfView = 80.0f;

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
}

void APrototypeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckRunState();

	if (!bIsRunning && GetVelocity().SizeSquared() > KINDA_SMALL_NUMBER)
	{
		FRotator TargetRotation = FRotator(0.0f, GetControlRotation().Yaw, 0.0f);

		FRotator CurrentRotation = GetActorRotation();

		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, WalkTurnRate);

		SetActorRotation(NewRotation);
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

	float FinalTargetZ = CurrentBaseCameraZ + ZOffsetBob;
	float FinalTargetY = 45.0f + YOffsetBob; // 45.0f는 기본 Y 오프셋

	CameraBoom->SocketOffset.Z = FMath::FInterpTo(CameraBoom->SocketOffset.Z, FinalTargetZ, DeltaTime, 10.0f);
	CameraBoom->SocketOffset.Y = FMath::FInterpTo(CameraBoom->SocketOffset.Y, FinalTargetY, DeltaTime, 10.0f);

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
	}

}

void APrototypeCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

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
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APrototypeCharacter::ToggleCrouch(const FInputActionValue& Value)
{
	// 달리고 있다면 앉기 불가능
	if (bIsRunning)
	{
		return;
	}

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

