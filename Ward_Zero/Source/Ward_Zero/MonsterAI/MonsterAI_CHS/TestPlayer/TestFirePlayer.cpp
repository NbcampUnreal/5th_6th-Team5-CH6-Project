#include "TestFirePlayer.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "EditorState/EditorState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

// 디폴트값 설정
ATestFirePlayer::ATestFirePlayer()
{
	// 이 캐릭터가 프레임마다 Tick()을 호출하도록 설정합니다.  이 설정이 필요 없는 경우 비활성화하면 퍼포먼스가 향상됩니다.
	PrimaryActorTick.bCanEverTick = false;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
 
}
 
// 게임 시작 또는 스폰 시 호출
void ATestFirePlayer::BeginPlay()
{
	Super::BeginPlay();
 
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext,0);
		}
	}
}

void ATestFirePlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller)
	{
		const FRotator& Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(ForwardDirection,MovementVector.Y);
		AddMovementInput(RightDirection,MovementVector.X);
	}
}

void ATestFirePlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookDirection = Value.Get<FVector2D>();
	if (Controller)
	{
		AddControllerYawInput(LookDirection.X);
		AddControllerPitchInput(LookDirection.Y);
	}
}

void ATestFirePlayer::Shoot()
{
	FVector CameraLocation;
	FRotator CameraRotation;
	GetActorEyesViewPoint(CameraLocation, CameraRotation);
    
	FVector End = CameraLocation + (CameraRotation.Vector() * 5000.0f);
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = false;
	Params.bReturnPhysicalMaterial = true;
	// 로그 확인
	UE_LOG(LogTemp, Warning, TEXT("Shoot - Start: %s, End: %s"), *CameraLocation.ToString(), *End.ToString());

	if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, End, ECC_Visibility, Params))
	{
		FName HitBone = HitResult.BoneName;
		UE_LOG(LogTemp, Warning, TEXT("Hit Bone: %s"), *HitBone.ToString());
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitActor->GetName());
			UGameplayStatics::ApplyPointDamage(
			HitResult.GetActor(),
			10.0f,
			CameraRotation.Vector(),
			HitResult,
			GetController(),
			this,
			TestDamageType
		);
			// 무언가 맞았을 때는 충돌 지점까지 빨간 선
			DrawDebugLine(GetWorld(), CameraLocation, HitResult.ImpactPoint, FColor::Red, false, 2.0f, 0, 2.0f);
			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.0f, 12, FColor::Red, false, 2.0f);
		}
		
	}
	else
	{
		// [수정] 아무것도 맞지 않았을 때는 미리 계산한 End 지점까지 초록 선
		DrawDebugLine(GetWorld(), CameraLocation, End, FColor::Green, false, 2.0f, 0, 2.0f);
	}
}

// 프레임마다 호출
void ATestFirePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
 
}
 
//함수 기능을 입력에 바인딩하기 위해 호출
void ATestFirePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered,this,&ATestFirePlayer::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered,this,&ATestFirePlayer::Look);
		EIC->BindAction(ShootAction,ETriggerEvent::Started,this,&ATestFirePlayer::Shoot);
	}
 
}