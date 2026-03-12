#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Status/PlayerStatusComponent.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "Character/Components/Interaction/InteractionComponent.h"
#include "Character/Components/QuickTurn/QuickTurnComponent.h"
#include "Character/Components/FlashLight/FlashlightComponent.h"
#include "Character/Components/FootStep/FootstepComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/Components/Camera/PlayerCameraComponent.h"
#include "Character/Data/Movement/CharacterMovementData.h"
#include "Character/Data/Status/CharacterStatusData.h"
#include "Character/Data/AnimData/CharacterAnimData.h"
#include "Character/Data/Combat/CharacterCombatData.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Weapon/Weapon.h"
#include "Weapon/WZ_HUD_DH.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Animation/PlayerAnimInstance.h"
#include "UI_KWJ/GameOver/GameOverSubsystem.h"
#include "UI_KWJ/Save/SaveSubsystem.h"
#include "UI_KWJ/WeaponUI/WeaponUISubsystem.h"
#include "UI_KWJ/Health/HealthVignetteWidget.h"
#include "Gimmic_CY/InteractionBase.h"

APrototypeCharacter::APrototypeCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Tags.AddUnique(TEXT("Player"));

	// 컴포넌트 생성
	StatusComp = CreateDefaultSubobject<UPlayerStatusComponent>(TEXT("StatusComp"));
	CombatComp = CreateDefaultSubobject<UPlayerCombatComponent>(TEXT("CombatComp"));
	InteractionComp = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComp"));
	QuickTurnComp = CreateDefaultSubobject<UQuickTurnComponent>(TEXT("QuickTurnComp"));
	FlashLightComp = CreateDefaultSubobject<UFlashlightComponent>(TEXT("FlashLightComp"));
	FootstepComp = CreateDefaultSubobject<UFootstepComponent>(TEXT("FootstepComp"));
	CustomCameraComp = CreateDefaultSubobject<UPlayerCameraComponent>(TEXT("CustomCameraComp"));

	// 카메라 설정 
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	CameraBoom->TargetArmLength = 180.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.0f, 35.0f, 10.0f);

	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	MainCamera->bUsePawnControlRotation = false;

	// 물리 설정 
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 240.0f, 0.0f);
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	if (CombatComp && CombatComp->PistolWeapon)
	{
		CombatComp->PistolWeapon->SetActorEnableCollision(false);
	}
}


void APrototypeCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 입력 컨텍스트 등록
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (auto* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext) Sub->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	// 스테미나/체력 데이터 에셋 값 연결 (원본 로직 복구)
	if (StatusData && StatusComp)
	{
		StatusComp->MaxHealth = StatusData->MaxHealth;
		StatusComp->CurrHealth = StatusData->MaxHealth;
		StatusComp->MaxStamina = StatusData->MaxStamina;
		StatusComp->CurrStamina = StatusData->MaxStamina;
		StatusComp->StaminaDrainRate = StatusData->StaminaDrainRate;
		// StatusComponent 내의 Tick에서 이 값들을 사용하도록 보장됨
	}

	// 데이터 에셋 적용 (속도 등)
	if (MovementData)
	{
		GetCharacterMovement()->MaxWalkSpeed = MovementData->WalkSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = MovementData->CrouchMovementSpeed;
		OriginalArmLength = MovementData->DefaultArmLength;
		// CameraConfig가 유효하다면 위 수치들을 덮어씌움
	}

	if (CustomCameraComp)
	{
		CustomCameraComp->Initialize(CameraBoom, MainCamera, CameraConfig);
	}

	if (CombatComp)
	{
		CombatComp->SetupCombat(MainCamera);
	}

	// 애니메이션 레이어 초기화
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		if (UnarmedLayer) AnimInst->LinkAnimClassLayers(UnarmedLayer);
	}
	if (HealthVignetteClass)
	{
		APlayerController* VignettePC = Cast<APlayerController>(Controller);
		if (VignettePC)
		{
			HealthVignetteWidget = CreateWidget<UHealthVignetteWidget>(VignettePC, HealthVignetteClass);
			if (HealthVignetteWidget)
			{
				HealthVignetteWidget->AddToViewport(0);
				StatusComp->OnHealthChanged.AddDynamic(
					HealthVignetteWidget, &UHealthVignetteWidget::OnHealthChanged);
			}
		}
	}
}

void APrototypeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CameraBoom || !MainCamera || !GetCharacterMovement()) return;

	CheckRunState();

	// 퀵턴 처리 
	if (QuickTurnComp && QuickTurnComp->IsQuickTurning()) return;

	// 손전등 실시간 갱신
	if (FlashLightComp && FlashLightComp->IsUsingFlashlight()) // 손전등 사용 중일때만 호출 
	{
		FlashLightComp->UpdateFlashlight(DeltaTime);
	}

	// 캐릭터 몸체 회전 (비조준 / 조준)
	UpdateBodyRotation(DeltaTime);

#if !UE_BUILD_SHIPPING // 디버깅용 빌드에서 제외
	if (GEngine)
	{
		if (StatusComp)
		{
			FString StatusMsg = FString::Printf(TEXT("HP: %.0f / Stamina: %.0f"), StatusComp->CurrHealth, StatusComp->CurrStamina);
			GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Green, StatusMsg);
		}
		if (CombatComp && CombatComp->GetEquippedWeapon())
		{
			FString AmmoMsg = FString::Printf(TEXT("Ammo: %d / %d"), CombatComp->GetEquippedWeapon()->GetCurrentAmmo(), CombatComp->GetEquippedWeapon()->GetReserveAmmo());
			GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::Yellow, AmmoMsg);
		}
	}
#endif
	if (CustomCameraComp)
	{
		CustomCameraComp->UpdateCamera(DeltaTime);
	}
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UWeaponUISubsystem* WeaponUI = LP->GetSubsystem<UWeaponUISubsystem>())
			{
				WeaponUI->UpdateWeaponStatus();
			}
		}
	}
}
#pragma region Input Biding
void APrototypeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EI->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Move);
		EI->BindAction(LookAction, ETriggerEvent::Triggered, this, &APrototypeCharacter::Look);
		EI->BindAction(RunAction, ETriggerEvent::Started, this, &APrototypeCharacter::StartRunning);
		EI->BindAction(RunAction, ETriggerEvent::Completed, this, &APrototypeCharacter::EndRunning);
		EI->BindAction(CrouchAction, ETriggerEvent::Started, this, &APrototypeCharacter::ToggleCrouch);
		EI->BindAction(InteractAction, ETriggerEvent::Started, this, &APrototypeCharacter::Interact);
		EI->BindAction(EquipAction, ETriggerEvent::Started, this, &APrototypeCharacter::ToggleEquip);
		EI->BindAction(AimAction, ETriggerEvent::Started, this, &APrototypeCharacter::StartAiming);
		EI->BindAction(AimAction, ETriggerEvent::Completed, this, &APrototypeCharacter::StopAiming);
		EI->BindAction(FireAction, ETriggerEvent::Started, this, &APrototypeCharacter::Fire);
		EI->BindAction(FireAction, ETriggerEvent::Completed, this, &APrototypeCharacter::StopFire);
		EI->BindAction(ReloadAction, ETriggerEvent::Started, this, &APrototypeCharacter::Reload);
		EI->BindAction(QuickTurnAction, ETriggerEvent::Started, this, &APrototypeCharacter::PerformQuickTurn180);
		EI->BindAction(FlashlightAction, ETriggerEvent::Started, this, &APrototypeCharacter::ToggleFlashLight);
		EI->BindAction(EquipSlot1Action, ETriggerEvent::Started, this, &APrototypeCharacter::SelectWeapon1);
		EI->BindAction(EquipSlot2Action, ETriggerEvent::Started, this, &APrototypeCharacter::SelectWeapon2);
	}
}
#pragma endregion

void APrototypeCharacter::Move(const FInputActionValue& Value)
{
	if (GetIsQuickTurning()) return;

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller == nullptr) return;

	// 원본의 장전 중 이동 제한 로직 복구
	if (bIsCrouched && GetIsReloading()) return;

	float SpeedModifier = 1.0f;
	// 장전 중이면서 달리는 중이 아니면 속도 50% 감소
	if (!bIsCrouched && GetIsReloading() && !bIsRunning)
	{
		SpeedModifier = 0.5f;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y * SpeedModifier);
	AddMovementInput(RightDirection, MovementVector.X * SpeedModifier);
}

void APrototypeCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxis = Value.Get<FVector2D>();
	// AimLookSensitivity가 0이면 화면이 안 돌아갑니다. 기본값 0.5f 확인!
	float Sensitivity = (GetIsAiming()) ? AimLookSensitivity : 1.0f;

	AddControllerYawInput(LookAxis.X * Sensitivity);
	AddControllerPitchInput(LookAxis.Y * Sensitivity);
}

void APrototypeCharacter::StartRunning(const FInputActionValue& Value)
{
	if (CombatComp && CombatComp->IsAiming()) return;

	if (StatusComp && !StatusComp->CanSprint()) return;
	if (bIsCrouched && GetIsReloading()) return;
	if (bIsRunning)
	{
		EndRunning(Value);
	}
	else
	{
		bIsRunning = true;
		GetCharacterMovement()->MaxWalkSpeed = MovementData->RunSpeed;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		CameraBoom->CameraLagSpeed = 10.0f;

		FlashLightComp->UpdateFlashlight(0.0f);//달릴 때는 손전등 소켓 갱신 
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
	GetCharacterMovement()->MaxWalkSpeed = MovementData->WalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = false; // Strafe 모드로 복구
	CameraBoom->CameraLagSpeed = 15.0f;

	FlashLightComp->UpdateFlashlight(0.0f); //달리기 종료 시 원래 소켓으로 복구
}

void APrototypeCharacter::ToggleCrouch(const FInputActionValue& Value)
{
	if (bIsRunning || GetIsReloading()) return;
	bIsCrouched ? UnCrouch() : Crouch();
}

void APrototypeCharacter::ToggleEquip(const FInputActionValue& Value)
{
	if (!CombatComp || GetIsReloading()) return;
	if (!CombatComp->GetEquippedWeapon()) return;

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!AnimInst) return;

	if (GetIsAiming())
	{
		StopAiming(Value);
	}

	bool bWillDraw = !CombatComp->IsWeaponDrawn();

	CombatComp->ToggleEquip(nullptr, AnimInst);

	if (FlashLightComp)
	{
		FlashLightComp->SetFlashlightOff();
	}

	TSubclassOf<UAnimInstance> TargetLayer = UnarmedLayer;
	if (bWillDraw)
	{
		TargetLayer = (CombatComp->GetCurrentWeaponIndex() == 1) ? PistolLayer : SMGLayer;
	}
	AnimInst->LinkAnimClassLayers(TargetLayer);

	if (FlashLightComp)
	{
		FlashLightComp->UpdateFlashlight(0.0f);
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UWeaponUISubsystem* WeaponUI = LP->GetSubsystem<UWeaponUISubsystem>())
			{
				if (CombatComp->IsWeaponDrawn())
				{
					WeaponUI->NotifyWeaponChanged(CombatComp->GetCurrentWeaponIndex(), true);
				}
				else
				{
					WeaponUI->NotifyWeaponHolstered();
				}
			}
		}
	}
}

void APrototypeCharacter::StartAiming(const FInputActionValue& Value)
{
	if (CombatComp && CombatComp->StartAiming())
	{
		// 캐릭터 회전 및 이동 속도 설정
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->MaxWalkSpeed = MovementData->WalkSpeed * 0.5f;

		// 카메라 설정 
		//CameraBoom->bInheritPitch = false;
		CameraBoom->bEnableCameraLag = false;
	/*	MainCamera->bUsePawnControlRotation = true;*/

		// 컨트롤러 시야각 제한 및 크로스헤어 표시
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (PC->PlayerCameraManager)
			{
				if (CombatComp) CombatComp->SetCameraPitchLimit(true);
			}
			if (AWZ_HUD_DH* HUD = Cast<AWZ_HUD_DH>(PC->GetHUD()))
			{
				HUD->SetCrosshairVisibility(true);
			}
		}
	}
}

void APrototypeCharacter::StopAiming(const FInputActionValue& Value)
{
	if (CombatComp) CombatComp->StopAiming();

	// 캐릭터 회전 및 이동 속도 
	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		float SafeSpeed = (MovementData->WalkSpeed > 0.0f) ? MovementData->WalkSpeed : 200.0f;
		MoveComp->MaxWalkSpeed = SafeSpeed;
	}

	// 카메라 설정 복구
	//CameraBoom->bInheritPitch = true;
	CameraBoom->bEnableCameraLag = true;

	if (MainCamera)
	{
		//MainCamera->bUsePawnControlRotation = false;
		MainCamera->SetRelativeRotation(FRotator::ZeroRotator);
	}

	// 컨트롤러 시야각 복구 및 크로스헤어 숨김
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			if (PC->PlayerCameraManager)
			{
				if (CombatComp) CombatComp->SetCameraPitchLimit(false);
			}
		}
		if (AWZ_HUD_DH* HUD = Cast<AWZ_HUD_DH>(PC->GetHUD()))
		{
			HUD->SetCrosshairVisibility(false);
		}
	}
}

void APrototypeCharacter::CheckRunState()
{
	if (bIsRunning)
	{
		if ((StatusComp && !StatusComp->CanSprint()) || GetVelocity().SizeSquared() <= 100.0f)
		{
			EndRunning(FInputActionValue());
		}
	}
}

void APrototypeCharacter::Fire(const FInputActionValue& Value)
{
	if (CombatComp)
	{
		CombatComp->StartFire(nullptr, GetMesh()->GetAnimInstance(), CombatData->FireCameraShake);
	}
}

void APrototypeCharacter::StopFire(const FInputActionValue& Value)
{
	if (CombatComp) CombatComp->StopFire();
}

void APrototypeCharacter::Reload(const FInputActionValue& Value)
{
	if (CombatComp)
	{
		CombatComp->Reload();
	}
}

void APrototypeCharacter::ToggleFlashLight(const FInputActionValue& Value)
{
	if (FlashLightComp)
	{
		FlashLightComp->ToggleFlashlight();
	}
}
void APrototypeCharacter::SelectWeapon1(const FInputActionValue& Value)
{
	if (GetIsReloading() || !CombatComp) return;

	// 조준 중이었다면 강제 해제
	if (GetIsAiming()) StopAiming(Value);

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!AnimInst) return;

	// 이미 권총을 들고 있는 상태라면 무기 집어넣기 (토글)
	if (CombatComp->GetCurrentWeaponIndex() == 1 && CombatComp->IsWeaponDrawn())
	{
		ToggleEquip(Value);
		return;
	}

	// 무기 교체 
	CombatComp->ChangeWeapon(1, AnimInst);

	// 무기 교체 시 손전등 내리고 끄기 
	if (FlashLightComp)
	{
		FlashLightComp->SetFlashlightOff();
	}

	// 레이어 연결 및 손전등 상태 갱신
	if (PistolLayer) AnimInst->LinkAnimClassLayers(PistolLayer);
	if (FlashLightComp) FlashLightComp->UpdateFlashlight(0.0f);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UWeaponUISubsystem* WeaponUI = LP->GetSubsystem<UWeaponUISubsystem>())
			{
				WeaponUI->NotifyWeaponChanged(1, true);
			}
		}
	}
}


void APrototypeCharacter::SelectWeapon2(const FInputActionValue& Value)
{
	if (GetIsReloading() || !CombatComp) return;

	// 조준 중이었다면 강제 해제
	if (GetIsAiming()) StopAiming(Value);

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!AnimInst) return;

	// 이미 SMG를 들고 있는 상태라면 무기 집어넣기 
	if (CombatComp->GetCurrentWeaponIndex() == 2 && CombatComp->IsWeaponDrawn())
	{
		ToggleEquip(Value);
		return;
	}

	// 무기 교체 
	CombatComp->ChangeWeapon(2, AnimInst);

	

	if (FlashLightComp)
	{
		FlashLightComp->SetFlashlightOff();
	}

	// 레이어 연결 및 손전등 상태 갱신
	if (SMGLayer) AnimInst->LinkAnimClassLayers(SMGLayer);
	if (FlashLightComp) FlashLightComp->UpdateFlashlight(0.0f);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UWeaponUISubsystem* WeaponUI = LP->GetSubsystem<UWeaponUISubsystem>())
			{
				WeaponUI->NotifyWeaponChanged(2, true);
			}
		}
	}
}
void APrototypeCharacter::Interact(const FInputActionValue& Value)
{
	//if (InteractionComp)
	//{
	//	InteractionComp->TryInteract();
	//}

	/*TArray<AActor*> OverlappedActors;
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
	}*/

	if (bIsRunning) return;

	FVector NewLocation = GetActorLocation();
	NewLocation.Z += 50.0f;

	FVector Start = NewLocation;
	FVector End = Start + this->GetActorForwardVector() * 100.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	float SweepRadius = 45.0f;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(SweepRadius);

	bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		SphereShape,
		Params
	);

	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 5.0f, 0, 2.0f);
	DrawDebugSphere(GetWorld(), End, SweepRadius, 16, bHit ? FColor::Green : FColor::Red, false, 5.0f);

	if (!bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interact] 라인트레이스 히트 없음"));
		return;
	}

	AActor* HitActor = Hit.GetActor();

	if (!HitActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interact] HitActor가 null"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Interact] 히트 액터: %s (클래스: %s)"),
		*HitActor->GetName(), *HitActor->GetClass()->GetName());

	if (HitActor->GetClass()->ImplementsInterface(UInteractionBase::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interact] → InteractionBase 인터페이스 있음"));

		bool bCan = IInteractionBase::Execute_CanBeInteracted(HitActor);
		UE_LOG(LogTemp, Warning, TEXT("[Interact] → CanBeInteracted: %s"), bCan ? TEXT("TRUE") : TEXT("FALSE"));

		if (bCan)
		{
			EInteractionType Type = IInteractionBase::Execute_GetInteractionType(HitActor);
			UE_LOG(LogTemp, Warning, TEXT("[Interact] → InteractionType: %d"), static_cast<int32>(Type));

			// 문일 때만 몽타주 재생
			if (Type == EInteractionType::Door)
			{
				PendingDoorActor = HitActor;
				if (AnimData && AnimData->OpenDoorMontage)
				{
					PlayAnimMontage(AnimData->OpenDoorMontage);
				}
			}

			// 모든 타입 공통: 상호작용 실행
			IInteractionBase::Execute_OnIneracted(HitActor, this);
		}
	}
	// 2순위: 태그 방식 — 세이브 포인트
	else if (HitActor->ActorHasTag(TEXT("SavePoint")))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interact] → SavePoint 태그 감지: %s"), *HitActor->GetName());

		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			ULocalPlayer* LP = PC->GetLocalPlayer();
			if (LP)
			{
				USaveSubsystem* SaveSub = LP->GetSubsystem<USaveSubsystem>();
				if (SaveSub)
				{
					SaveSub->ShowSaveUI();
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Interact] → InteractionBase 인터페이스 없음, 태그도 없음"));
	}

}
void APrototypeCharacter::PerformQuickTurn180()
{
	if (QuickTurnComp)
	{
		QuickTurnComp->StartQuickTurn180();
	}
}

void APrototypeCharacter::PlayFootstepSound(FName FootBoneName)
{
	if (FootstepComp) FootstepComp->PlayFootstep(FootBoneName);
}

void APrototypeCharacter::OnDeath()
{
	bIsRunning = false;
	if (QuickTurnComp) QuickTurnComp->StopQuickTurn();

	if (CombatComp)
	{
		CombatComp->StopFire();
		CombatComp->StopAiming();
	}

	// 입력 컴포넌트 비활성화 및 UI 모드 전환
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		DisableInput(PC);
		PC->SetShowMouseCursor(true);

		FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);
	}

	// 물리 및 충돌 설정 (래그돌)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);

	// 게임 오버 UI 호출 타이머
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [this]()
		{
			if (APlayerController* PC = Cast<APlayerController>(Controller))
			{
				if (ULocalPlayer* LP = PC->GetLocalPlayer())
				{
					if (UGameOverSubsystem* GameOverSys = LP->GetSubsystem<UGameOverSubsystem>())
					{
						GameOverSys->ShowGameOver();
					}
				}
			}
		}, 2.0f, false);
}

float APrototypeCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!StatusComp || StatusComp->IsDead()) return 0.0f;

	// 데미지 적용 (HP 감소 및 사망 판정)
	float ActualDamage = StatusComp->ApplyDamage(DamageAmount);

	// AI 위치 파악
	FVector AttackerLocation;
	bool bAttackerFound = false;

	if (EventInstigator && EventInstigator->GetPawn())
	{
		AttackerLocation = EventInstigator->GetPawn()->GetActorLocation();
		bAttackerFound = true;
	}
	else if (DamageCauser)
	{
		AttackerLocation = DamageCauser->GetActorLocation();
		bAttackerFound = true;
	}

	// 방향 벡터 계산 
	FVector ToAttackerDir;
	if (bAttackerFound)
	{
		ToAttackerDir = (AttackerLocation - GetActorLocation());
	}
	else
	{
		ToAttackerDir = GetActorForwardVector();
	}

	ToAttackerDir.Z = 0.0f;
	ToAttackerDir.Normalize();

	if (!StatusComp->IsDead())
	{
		PlayHitReaction(ToAttackerDir);
	}
	else
	{
		PlayDeathReaction(ToAttackerDir);
	}

	return ActualDamage;
}

EPlayerHitDirection APrototypeCharacter::GetHitDirection(const FVector& ToAttackerDir)
{
	if (ToAttackerDir.IsNearlyZero()) return EPlayerHitDirection::Front; // 기본값

	// 캐릭터의 앞/오른쪽 벡터와 공격자 방향의 내적 계산
	float ForwardDot = FVector::DotProduct(GetActorForwardVector(), ToAttackerDir);
	float RightDot = FVector::DotProduct(GetActorRightVector(), ToAttackerDir);

	EPlayerHitDirection Result;

	// 0.707 = 45도 
	if (ForwardDot >= 0.5f) Result = EPlayerHitDirection::Front;
	else if (ForwardDot <= -0.5f) Result = EPlayerHitDirection::Back;
	else if (RightDot >= 0.f) Result = EPlayerHitDirection::Right;
	else Result = EPlayerHitDirection::Left;

	UE_LOG(LogTemp, Log, TEXT("Hit Direction: %d (F_Dot: %f, R_Dot: %f)"), (int32)Result, ForwardDot, RightDot);

	return Result;
}

void APrototypeCharacter::PlayHitReaction(const FVector& ToAttackerDir)
{
	if (!AnimData || !CombatComp) return;

	if (CombatComp->GetIsReloading())
	{
		CombatComp->CancelReload(GetMesh()->GetAnimInstance());
	}

	EPlayerHitDirection HitDir = GetHitDirection(ToAttackerDir);
	bool bIsDrawn = CombatComp->IsWeaponDrawn();
	int32 WeaponIdx = CombatComp->GetCurrentWeaponIndex();

	TMap<EPlayerHitDirection, TObjectPtr<UAnimMontage>>* TargetMap = &AnimData->UnarmedHitMontages;

	if (bIsDrawn) {
		if (WeaponIdx == 1) TargetMap = &AnimData->PistolHitMontages;
		else if (WeaponIdx == 2) TargetMap = &AnimData->SMGHitMontages;
	}

	if (TargetMap && TargetMap->Contains(HitDir))
	{
		PlayAnimMontage((*TargetMap)[HitDir]);
	}
}
void APrototypeCharacter::PlayDeathReaction(const FVector& ToAttackerDir)
{
	EPlayerHitDirection HitDir = GetHitDirection(ToAttackerDir);
	UAnimMontage* MontageToPlay = (AnimData) ? AnimData->DeathMontages.FindRef(HitDir) : nullptr;

	if (MontageToPlay)
	{
		float Duration = PlayAnimMontage(MontageToPlay);

		// 몽타주가 끝날 때쯤 OnDeath(래그돌)를 호출하도록 타이머 설정
		FTimerHandle DeathTimer;
		GetWorldTimerManager().SetTimer(DeathTimer, this, &APrototypeCharacter::OnDeath, Duration * 0.8f, false);
	}
	else
	{
		// 재생할 몽타주가 없다면 즉시 OnDeath
		OnDeath();
	}
}

void APrototypeCharacter::UpdateBodyRotation(float DeltaTime)
{
	// 캐릭터 몸체 회전 (비조준 / 조준)
	if (CombatComp && CombatComp->IsAiming())
	{
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		bUseControllerRotationYaw = false;
		if (bIsRunning)
		{
			GetCharacterMovement()->bOrientRotationToMovement = true;
		}
		else
		{
			GetCharacterMovement()->bOrientRotationToMovement = false;
			if (GetVelocity().SizeSquared() > 100.0f)
			{
				FRotator TargetRot = FRotator(0.f, GetControlRotation().Yaw, 0.f);
				FRotator CurrentRot = GetActorRotation();
				FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 5.0f);
				SetActorRotation(NewRot);
			}
		}
	}
}

// 인터페이스 함수 구현
bool APrototypeCharacter::GetIsPistolEquipped() const { return CombatComp && CombatComp->IsPistolEquipped(); }
bool APrototypeCharacter::GetIsGround() const { return GetCharacterMovement()->IsMovingOnGround(); }
bool APrototypeCharacter::GetIsQuickTurning() const { return QuickTurnComp && QuickTurnComp->IsQuickTurning(); }
int32 APrototypeCharacter::GetTurnIndex() const { return QuickTurnComp ? QuickTurnComp->GetTurnIndex() : 0; }
FVector APrototypeCharacter::GetHandIKTargetLoc() const { return CombatComp ? CombatComp->GetHandIKTarget() : FVector::ZeroVector; }
bool APrototypeCharacter::GetIsAiming() const { return CombatComp && CombatComp->IsAiming(); }
void APrototypeCharacter::SetIsQuickTurning(bool bIsTurning) { if (QuickTurnComp && !bIsTurning) QuickTurnComp->StopQuickTurn(); }
AWeapon* APrototypeCharacter::GetEquippedWeapon() { return CombatComp ? CombatComp->GetEquippedWeapon() : nullptr; }
bool APrototypeCharacter::GetIsReloading() const { return CombatComp && CombatComp->GetIsReloading(); }
bool APrototypeCharacter::GetIsUseFlashLight() const { return FlashLightComp ? FlashLightComp->IsUsingFlashlight() : false; }
bool APrototypeCharacter::GetIsSMGEquipped() const { return CombatComp && (CombatComp->GetCurrentWeaponIndex() == 2) && CombatComp->IsWeaponDrawn(); }
int32 APrototypeCharacter::GetCurrentWeaponIndex() const { return CombatComp ? CombatComp->GetCurrentWeaponIndex() : 0; }
float APrototypeCharacter::GetAimPitch() const { return CombatComp ? CombatComp->GetAimPitch() : 0.0f; }
float APrototypeCharacter::GetAimYaw() const { return CombatComp ? CombatComp->GetAimYaw() : 0.0f; }
bool APrototypeCharacter::IsFiring() const { return CombatComp && CombatComp->IsFiring(); }
float APrototypeCharacter::GetCurrSpread() const { return CombatComp ? CombatComp->CurrentSpread : 0.0f; }
UPlayerCombatComponent* APrototypeCharacter::GetCombatComp() const { return CombatComp ? CombatComp : nullptr; }
bool APrototypeCharacter::IsEquipping() const
{
	if (UAnimInstance* AI = GetMesh()->GetAnimInstance())
		return AI->Montage_IsPlaying(nullptr);
	return false;
}
USkeletalMeshComponent* APrototypeCharacter::GetEquippedWeaponMesh()
{
	if (CombatComp)
	{
		AWeapon* CurrentWeapon = CombatComp->GetEquippedWeapon();
		if (IsValid(CurrentWeapon)) // 무기가 실제로 스폰되어 있는지 확인
		{
			return CurrentWeapon->WeaponMesh;
		}
	}
	return nullptr;
}