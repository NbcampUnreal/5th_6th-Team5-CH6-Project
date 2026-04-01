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
#include "UI_KWJ/PauseMenu/PauseMenuSubsystem.h"
#include "Engine/OverlapResult.h"
#include "Gimmic_CY/Interface/InteractionBase.h"
#include "Gimmic_CY/Object/Lever/Lever.h"
#include "MotionWarpingComponent.h"
#include "Components/TimelineComponent.h"

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
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->ProbeSize = 15.0f;
	
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	MainCamera->bUsePawnControlRotation = false;

	// 물리 설정 
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 240.0f, 0.0f);
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	if (CombatComp && CombatComp->PistolWeapon)
	{
		CombatComp->PistolWeapon->SetActorEnableCollision(false);
	}

	MotionWarpingComp = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComp"));
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
		GetCharacterMovement()->MaxAcceleration = MovementData->MaxAcceleration;
		GetCharacterMovement()->BrakingDecelerationWalking = MovementData->BrakingDeceleration;
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

	if (InteractionComp)
	{
		InteractionComp->Initialize(this);
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
	if (GetCharacterMovement())
	{
		// 카메라 방향이 아닌 이동 방향을 바라보게 설정
		GetCharacterMovement()->bOrientRotationToMovement = true;

		// 컨트롤러 회전(카메라)에 몸을 고정하지 않음
		bUseControllerRotationYaw = false;
	}
	if (CustomCameraComp)
	{
		// 캐릭터의 계산이 다 끝난 "다음에" 카메라 틱을 돌리라고 명시하는 것
		CustomCameraComp->AddTickPrerequisiteActor(this);
	}
	/*if (ReviveCameraCurve)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("UpdateReviveCamera"));
		ReviveCameraTimeline.AddInterpFloat(ReviveCameraCurve, TimelineProgress);
	}*/
}

void APrototypeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsRunning) CheckRunState();

	UPlayerAnimInstance* AnimInst = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	bool bIsTurningNow = (AnimInst && AnimInst->bIsTurn);

	if (bIsTurningNow || GetIsQuickTurning() || GetIsInteracting())
	{
		bUseControllerRotationYaw = false;
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->bOrientRotationToMovement = false;
			GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 0.f);
		}
	}
	else
	{
		// 턴 중이 아닐 때의 로직
		if (GetIsAiming())
		{
			bUseControllerRotationYaw = true;
			GetCharacterMovement()->bOrientRotationToMovement = false;
		}
		else
		{
			bUseControllerRotationYaw = false;
			GetCharacterMovement()->bOrientRotationToMovement = true;
			GetCharacterMovement()->RotationRate = FRotator(0.0f, 240.0f, 0.0f);
		}
	}
	if (MainCamera && GetMesh())
	{
		// 카메라와 캐릭터 메쉬 중심 간의 거리 계산
		float DistanceToCamera = FVector::Dist(MainCamera->GetComponentLocation(), GetMesh()->GetComponentLocation());

		// 거리가 60 미만으로 너무 가까워지면 메쉬를 숨김 (숫자는 테스트하며 조절)
		GetMesh()->SetOwnerNoSee(DistanceToCamera < 60.0f);
	}

	UpdateBodyRotation(DeltaTime);
	/*ReviveCameraTimeline.TickTimeline(DeltaTime);*/
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
		EI->BindAction(HealAction, ETriggerEvent::Started, this, &APrototypeCharacter::StartHeal);
		EI->BindAction(PauseAction, ETriggerEvent::Started, this, &APrototypeCharacter::TogglePauseMenu);
	}
}
#pragma endregion

void APrototypeCharacter::Move(const FInputActionValue& Value)
{
	if (GetIsQuickTurning()) return;

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller == nullptr) return;

	// 앉아서 장전 중 이동 제한 
	if (bIsCrouched && GetIsReloading()) return;

	float SpeedModifier = 1.0f;

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
	if (GetIsInteracting()) return;
	if (bIsInVent) return;

	FVector LastInput = GetLastMovementInputVector();
	FVector LocalInput = GetActorRotation().UnrotateVector(LastInput);
	if (LocalInput.X < -0.2f) return;

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
		FlashLightComp->UpdateFlashlight(0.0f);
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
	GetCharacterMovement()->bOrientRotationToMovement = true;
	/*GetCharacterMovement()->bOrientRotationToMovement = false;*/ // Strafe 모드로 복구
	FlashLightComp->UpdateFlashlight(0.0f); //달리기 종료 시 원래 소켓으로 복구
}

void APrototypeCharacter::ToggleCrouch(const FInputActionValue& Value)
{
	if (bIsInVent && bIsCrouched) return;
	if (bIsRunning || GetIsReloading() || IsEquipping()) return;
	if (!bIsCrouched && GetIsAiming() && !GetIsSMGEquipped())
	{
		StopAiming(Value);
	}

	bIsCrouched ? UnCrouch() : Crouch();
}

void APrototypeCharacter::ToggleEquip(const FInputActionValue& Value)
{
	if (!CombatComp || GetIsReloading()) return;
	if (!CombatComp->GetEquippedWeapon()) return;
	if (GetIsInteracting()) return;

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

		CurrentLayerType = (CombatComp->GetCurrentWeaponIndex() == 1) ? EWeaponLayerType::Pistol : EWeaponLayerType::SMG;
	}
	else
	{
		CurrentLayerType = EWeaponLayerType::Unarmed;
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
	if (bIsInVent) return;
	if (bIsCrouched && !GetIsSMGEquipped()) return;
	if (!GetbIsWeaponDrawn()) return;
	if (CombatComp && CombatComp->StartAiming())
	{
		// 캐릭터 회전 및 이동 속도 설정
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->MaxWalkSpeed = MovementData->WalkSpeed * 0.5f;

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
	if (!GetIsAiming()) return;
	if (CombatComp) CombatComp->StopAiming();
	// 캐릭터 회전 및 이동 속도 
	bUseControllerRotationYaw = false;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = true; // Walk일때도 이동 방향으로 회전 
		float SafeSpeed = (MovementData->WalkSpeed > 0.0f) ? MovementData->WalkSpeed : 200.0f;
		MoveComp->MaxWalkSpeed = SafeSpeed;
	}

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
		FVector AccelNormal = GetCharacterMovement()->GetCurrentAcceleration().GetSafeNormal2D();

		if (AccelNormal.IsNearlyZero())
		{
			EndRunning(FInputActionValue());
			return;
		}

		float ForwardDot = FVector::DotProduct(GetActorForwardVector(), AccelNormal);
		if (ForwardDot < -0.3f)
		{
			EndRunning(FInputActionValue());
		} //107도 
	}
}

void APrototypeCharacter::Fire(const FInputActionValue& Value)
{
	if (StatusComp && StatusComp->IsDead()) return;
	if (bIsCrouched && !GetIsSMGEquipped()) return;
	if (bIsInVent) return;
	if (CombatComp)
	{
		TSubclassOf<UCameraShakeBase> FinalCamShake = nullptr;

		if (AWeapon* CurrentWeapon = CombatComp->GetEquippedWeapon())
		{
			FinalCamShake = CurrentWeapon->GetFireCameraShake();
		}

		if (!FinalCamShake && CombatData)
		{
			FinalCamShake = CombatData->FireCameraShake;
		}

		CombatComp->StartFire(nullptr, GetMesh()->GetAnimInstance(), FinalCamShake);
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
	SwitchWeaponByIndex(1);
}

void APrototypeCharacter::SelectWeapon2(const FInputActionValue& Value)
{
	SwitchWeaponByIndex(2);
}

void APrototypeCharacter::TogglePauseMenu(const FInputActionValue& Value)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UPauseMenuSubsystem* PauseSubsystem = LP->GetSubsystem<UPauseMenuSubsystem>())
			{
				AbortAllActions();
				PauseSubsystem->TogglePauseMenu();
			}
		}
	}
}

void APrototypeCharacter::Interact(const FInputActionValue& Value)
{
	if (bIsRunning || GetIsAiming()) return;

	if (InteractionComp)
	{
		InteractionComp->TryInteract();
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

	AbortAllActions(); // 모든 진행 중인 행동 강제 종료

	// 물리 및 충돌 설정 (래그돌)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);

	// 입력 컴포넌트 비활성화 및 UI 모드 전환
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		DisableInput(PC);
		PC->SetShowMouseCursor(true);

		FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);

		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UGameOverSubsystem* GameOverSystem = LocalPlayer->GetSubsystem<UGameOverSubsystem>())
			{
				GameOverSystem->ShowGameOver();
			}
		}
	}
}

float APrototypeCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!StatusComp || StatusComp->IsDead()) return 0.0f;

	bool bWasInteracting = GetIsInteracting(); 
	if (bWasInteracting) // 피격 시 상호작용 중단 
	{
		// 아직 손에 붙지 않은 아이템이면 월드에 복구
		if (InteractionComp && InteractionComp->CurrentInteractingItem)
		{
			AActor* Item = InteractionComp->CurrentInteractingItem;
			// Attach되지 않은 상태면(픽업 중간) 아이템 콜리전/상호작용 복구
			if (Item->GetAttachParentActor() != this)
			{
				Item->SetActorEnableCollision(true);
				if (IInteractionBase* Interactable = Cast<IInteractionBase>(Item))
					Interactable->SetBCanInteract(true);
			}
		}
		AbortAllActions(); // 몽타주 중단 + 입력 복구 + 인터렉션 상태 초기화
	}

	// 데미지 적용 (HP 감소 및 사망 판정)
	float ActualDamage = StatusComp->ApplyDamage(DamageAmount);

	// 피격 사운드 재생
	if (!StatusComp->IsDead() && StatusData && StatusData->HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, StatusData->HitSound, GetActorLocation());
	}

	// 피격 화면 흔들림
	if (HitCameraShakeClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->ClientStartCameraShake(HitCameraShakeClass);
		}
	}

	// 방향 벡터 계산 (분리된 헬퍼 함수 호출)
	FVector ToAttackerDir = GetAttackerDirection(EventInstigator, DamageCauser);

	// 애니메이션 리액션 재생
	if (!StatusComp->IsDead()) PlayHitReaction(ToAttackerDir);
	else PlayDeathReaction(ToAttackerDir);

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
	// 사망 사운드 재생
	if (StatusData && StatusData->DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, StatusData->DeathSound, GetActorLocation());
	}
	EPlayerHitDirection HitDir = GetHitDirection(ToAttackerDir);
	UAnimMontage* MontageToPlay = (AnimData) ? AnimData->DeathMontages.FindRef(HitDir) : nullptr;

	if (MontageToPlay)
	{
		float Duration = PlayAnimMontage(MontageToPlay);
		float SafeWaitTime = (Duration > 0.1f) ? Duration * 0.8f : 1.5f;

		FTimerHandle DeathTimer;
		GetWorldTimerManager().SetTimer(DeathTimer, this, &APrototypeCharacter::OnDeath, SafeWaitTime, false);
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
	bool bIsAiming = CombatComp && CombatComp->IsAiming();

	// 조준 중이거나 달리는 중이면 물리 컴포넌트(CharacterMovement)가 자동 처리하므로 연산 패스
	if (bIsAiming || bIsRunning) return;
}

void APrototypeCharacter::Revive()
{
	if (!StatusComp || !StatusComp->IsDead()) return;

	StatusComp->ReviveStatus(1.0f); // 1.0 => 100% 체력 부활

	// 만약 레그돌이면 레그돌 해제 
	GetMesh()->SetSimulatePhysics(false);

	// 콜리전 원상 복구 
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Player"));

	// 레그돌 해제 시 메시 위치가 틀어질 경우 위치 재조정 
	GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 무기/조준/달리기 상태 변수 초기화 
	bIsRunning = false;
	if (CombatComp)
	{
		CombatComp->StopFire();
		CombatComp->StopAiming();
	}

	// UI 모드를 게임 모드로 변경 (입력은 아직 안 켭니다!)
	APlayerController* PC = Cast<APlayerController>(Controller);
	if (PC)
	{
		PC->SetShowMouseCursor(false);

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);

		// GameOver UI 숨기기 
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UGameOverSubsystem* GameOverSystem = LocalPlayer->GetSubsystem<UGameOverSubsystem>())
			{
				GameOverSystem->HideGameOver();
			}
		}
	}

	// 몽타주 재생 및 길이(시간) 가져오기
	float MontageDuration = 0.0f;
	if (StatusComp->ReviveMontage)
	{
		MontageDuration = PlayAnimMontage(StatusComp->ReviveMontage);
	}

	// 몽타주가 끝나고 나서 입력받게 세팅 
	if (MontageDuration > 0.0f)
	{
		FTimerHandle ReviveInputTimer;
		GetWorldTimerManager().SetTimer(ReviveInputTimer, [this]()
			{
				if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
				{
					EnableInput(PlayerController);
				}
			}, MontageDuration, false); // 몽타주 길이만큼 딜레이
	}
	else
	{
		if (PC) EnableInput(PC);
	}
}
void APrototypeCharacter::StartHeal()
{
	if (!StatusComp || StatusComp->IsDead()) return;
	if (StatusComp->HealingItemCount <= 0) return;
	if (GetIsReloading() || GetIsAiming() || IsEquipping() || GetIsInteracting()) return;
	if (StatusComp->CurrHealth >= StatusComp->MaxHealth) return;
	if (bIsUseHeal) return;

	bIsUseHeal = true;
	if (AnimData && AnimData->HealMontage)
	{
		if (CombatComp && CombatComp->IsWeaponDrawn())
		{
			bWasWeaponDrawnBeforeHeal = true;
			CombatComp->HandleWeaponAttachment(false);
		}

		if (CameraBoom)
			CameraBoom->bDoCollisionTest = false;

		// 몽타주 종료 시 무기 복구 바인딩
		if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
			AnimInst->OnMontageEnded.AddDynamic(this, &APrototypeCharacter::OnHealMontageEnded);

		PlayAnimMontage(AnimData->HealMontage);
		StatusComp->AddHealingItem(-1);
		if (CombatComp) CombatComp->StopFire();
	}
}
void APrototypeCharacter::OnHealPoint()
{
	if (StatusComp)
	{
		StatusComp->Heal(StatusData->HealAmount);
	}
}
void APrototypeCharacter::SpawnHealItemVisual() { if (StatusComp) StatusComp->SpawnHealItemVisual(GetMesh()); }
void APrototypeCharacter::DestroyHealItemVisual() { if (StatusComp) StatusComp->DestroyHealItemVisual(); bIsUseHeal = false; }
void APrototypeCharacter::PopHealItemCap() { if (StatusComp) StatusComp->PopHealItemCap(); }

void APrototypeCharacter::PlayPickupAnimation(AActor* TargetItem)
{
}

void APrototypeCharacter::AttachInteractingItem()
{
	if (InteractionComp)
	{
		InteractionComp->AttachInteractingItem();
	}
}

void APrototypeCharacter::ConsumeInteractingItem()
{
	if (InteractionComp)
	{
		InteractionComp->ConsumeInteractingItem();
	}
}

void APrototypeCharacter::SwitchWeaponByIndex(int32 WeaponIndex)
{
	if (GetIsReloading() || !CombatComp) return;
	if (GetIsInteracting()) return;
	if (GetIsAiming()) StopAiming(FInputActionValue());

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!AnimInst) return;

	// 이미 해당 무기를 들고 있는 상태라면 무기 집어넣기 (토글)
	if (CombatComp->GetCurrentWeaponIndex() == WeaponIndex && CombatComp->IsWeaponDrawn())
	{
		ToggleEquip(FInputActionValue());
		return;
	}
	// 무기 교체 
	CombatComp->ChangeWeapon(WeaponIndex, AnimInst);

	// 무기 교체 시 손전등 내리고 끄기 
	if (FlashLightComp) FlashLightComp->SetFlashlightOff();

	// 레이어 연결 및 손전등 상태 갱신
	TSubclassOf<UAnimInstance> TargetLayer = (WeaponIndex == 1) ? PistolLayer : SMGLayer;

	CurrentLayerType = (WeaponIndex == 1) ? EWeaponLayerType::Pistol : EWeaponLayerType::SMG;

	if (TargetLayer) AnimInst->LinkAnimClassLayers(TargetLayer);

	if (FlashLightComp) FlashLightComp->UpdateFlashlight(0.0f);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UWeaponUISubsystem* WeaponUI = LP->GetSubsystem<UWeaponUISubsystem>())
			{
				WeaponUI->NotifyWeaponChanged(WeaponIndex, true);
			}
		}
	}
}

FVector APrototypeCharacter::GetAttackerDirection(AController* EventInstigator, AActor* DamageCauser)
{
	FVector AttackerLocation;

	if (EventInstigator && EventInstigator->GetPawn())
	{
		AttackerLocation = EventInstigator->GetPawn()->GetActorLocation();
	}

	else if (DamageCauser)
	{
		AttackerLocation = DamageCauser->GetActorLocation();
	}
	else
	{
		return GetActorForwardVector();
	}

	FVector Dir = (AttackerLocation - GetActorLocation());
	Dir.Z = 0.0f;
	return Dir.GetSafeNormal();
}

#pragma region 인터페이스 구현 
void APrototypeCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (CombatComp && CombatComp->IsWeaponDrawn())
	{
		CombatComp->HandleWeaponAttachment(true);
	}
}

void APrototypeCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (CombatComp && CombatComp->IsWeaponDrawn())
	{
		CombatComp->HandleWeaponAttachment(true);
	}
}

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
bool APrototypeCharacter::GetIsInVent() const { return bIsInVent; }
bool APrototypeCharacter::GetIsUseHeal() const { return bIsUseHeal; }

bool APrototypeCharacter::GetbIsWeaponDrawn() const
{
	return CombatComp ? CombatComp->IsWeaponDrawn() : false;
}

bool APrototypeCharacter::GetIsInjured() const { return StatusComp ? StatusComp->IsInjured() : false; }

void APrototypeCharacter::ExecuteHealPoint()
{
	if (StatusComp)
	{
		StatusComp->Heal(30.0f);
	}
}

bool APrototypeCharacter::GetIsInteracting() const
{
	return bIsInteractingDoor || (InteractionComp && InteractionComp->CurrentInteractingItem != nullptr);
}

bool APrototypeCharacter::IsEquipping() const
{
	if (!CombatComp) return false;

	if (UAnimInstance* AI = GetMesh()->GetAnimInstance())
	{
		// 아무 몽타주나 검사하는 것이 아니라, 실제 '장착/해제 몽타주' 중 하나가 재생 중일 때만 true 반환!
		return AI->Montage_IsPlaying(CombatComp->Pistol_EquipMontage) ||
			AI->Montage_IsPlaying(CombatComp->Pistol_UnEquipMontage) ||
			AI->Montage_IsPlaying(CombatComp->SMG_EquipMontage) ||
			AI->Montage_IsPlaying(CombatComp->SMG_UnEquipMontage);
	}
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

void APrototypeCharacter::SetDoorPasscode(int32 Passcode)
{
	//if (IsValid(PlayerHUD))
	//{
	//	PlayerHUD->SetPasscode(Passcode, Door, PC);
	//}
}

void APrototypeCharacter::AbortAllActions()
{
	if (InteractionComp && InteractionComp->CurrentInteractingItem != nullptr)
	{
		AActor* Item = InteractionComp->CurrentInteractingItem;

		if (Item->GetAttachParentActor() == this)
		{
			// Attach까지 된 상태 => 그냥 픽업 완료 처리
			InteractionComp->ConsumeInteractingItem();
		}
		else
		{
			// Attach안된 상태 => 콜리전 + bCanInteract 둘 다 복구
			Item->SetActorEnableCollision(true);
			if (IInteractionBase* Interactable = Cast<IInteractionBase>(Item))
			{
				Interactable->SetBCanInteract(true);
			}
		}
	}
	// 모든 몽타주 중단
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		AnimInst->Montage_Stop(0.2f);
	}
	DestroyHealItemVisual();
	// 인터렉션 상태 강제 종료
	if (InteractionComp)
	{
		InteractionComp->EndInteraction();
		InteractionComp->bIsInteractingDoor = false;
		InteractionComp->CurrentInteractingItem = nullptr;
	}

	// 전투 관련 상태 초기화
	CombatComp->StopFire();
	if (CombatComp->IsAiming()) CombatComp->StopAiming();
	if (CombatComp->GetEquippedWeapon())
	{
		CombatComp->GetEquippedWeapon()->SetIsReloading(false);
	}

	// 루트 모션 및 이동 잠금 해제
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}
	if (MotionWarpingComp)
	{
		MotionWarpingComp->RemoveAllWarpTargets();
	}
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		EnableInput(PC);
		PC->ResetIgnoreInputFlags();
	}
}

void APrototypeCharacter::UpdateVentState()
{
	bool bNowInVent = (VentOverlapCount > 0);

	if (bIsInVent != bNowInVent)
	{
		bIsInVent = bNowInVent;

		// 캐릭터 본체 메시 숨기기 (카메라에만 안 보이게)
		if (GetMesh())
		{
			GetMesh()->SetOwnerNoSee(bIsInVent);
		}

		// 무기 메시 업데이트
		if (CombatComp)
		{
			CombatComp->HandleWeaponAttachment(CombatComp->IsWeaponDrawn());
		}
	}
}

void APrototypeCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (OtherActor && OtherActor->ActorHasTag(TEXT("VentArea")))
	{
		VentOverlapCount++;
		UpdateVentState();
	}
}

void APrototypeCharacter::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);
	if (OtherActor && OtherActor->ActorHasTag(TEXT("VentArea")))
	{
		VentOverlapCount = FMath::Max(0, VentOverlapCount - 1);
		UpdateVentState();
	}
}

void APrototypeCharacter::ChangeLocomotionCameraShake(int32 StateIndex)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->PlayerCameraManager) return;

	// 기존 쉐이크 중지
	if (CurrentCameraShake)
	{
		PC->PlayerCameraManager->StopCameraShake(CurrentCameraShake, false);
	}

	TSubclassOf<UCameraShakeBase> ShakeToPlay = nullptr;
	switch (StateIndex)
	{
	case 0: ShakeToPlay = IdleShakeClass; break;
	case 1: ShakeToPlay = WalkShakeClass; break;
	case 2: ShakeToPlay = RunShakeClass; break;
	}

	if (ShakeToPlay)
	{
		CurrentCameraShake = PC->PlayerCameraManager->StartCameraShake(ShakeToPlay, 1.0f);
	}
}

void APrototypeCharacter::OnHealMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!AnimData || Montage != AnimData->HealMontage) return;

	// 델리게이트 해제
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
		AnimInst->OnMontageEnded.RemoveDynamic(this, &APrototypeCharacter::OnHealMontageEnded);

	DestroyHealItemVisual();

	// 이전에 무기를 들고 있었다면 복구
	if (bWasWeaponDrawnBeforeHeal && CombatComp)
		CombatComp->HandleWeaponAttachment(true);

	bWasWeaponDrawnBeforeHeal = false;
}
