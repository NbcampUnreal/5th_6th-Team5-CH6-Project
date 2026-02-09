#include "Prototype_Character/PrototypeCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Objects/Ladder.h"
#include "Objects/Interface/Interact.h"
#include "Components/BoxComponent.h"

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
	if (bIsQuickTurning)
	{
		return;
	}
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
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
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