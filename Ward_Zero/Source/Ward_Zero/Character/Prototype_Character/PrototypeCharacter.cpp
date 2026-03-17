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
#include "Engine/OverlapResult.h"
#include "Gimmic_CY/HealItemActor.h"
#include "Components/WidgetComponent.h"
#include "MotionWarpingComponent.h"

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

	// 장전 중이면서 달리는 중이 아니면 속도 50% 감소
	if (GetIsReloading())
	{
		SpeedModifier = 0.6f;
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

	if (GetIsReloading()) return;

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
	GetCharacterMovement()->bOrientRotationToMovement = false; // Strafe 모드로 복구

	FlashLightComp->UpdateFlashlight(0.0f); //달리기 종료 시 원래 소켓으로 복구
}

void APrototypeCharacter::ToggleCrouch(const FInputActionValue& Value)
{
	if (bIsRunning || GetIsReloading() || IsEquipping()) return;
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
		if (bIsRunning)
		{
			EndRunning(Value);
		}
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

	GetCharacterMovement()->MaxWalkSpeed *= MovementData->multiplySMGRunSpeed;

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

	GetCharacterMovement()->MaxWalkSpeed = MovementData->WalkSpeed * 0.9f;

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

	/*FVector Start = NewLocation;
	FVector End = Start + this->GetActorForwardVector() * 80.f;*/
	FVector Start = MainCamera->GetComponentLocation(); // 카메라 위치에서 시작
	FVector End = Start + (MainCamera->GetForwardVector() * 250.0f);

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	float SphereRadius = 45.0f;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(SphereRadius);

	bool bOverlap = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		End,               // 구체의 중심 위치
		FQuat::Identity,
		ECC_Visibility,
		SphereShape,
		Params
	);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.0f, 0, 1.0f);
	DrawDebugSphere(GetWorld(), End, SphereRadius, 16, bOverlap ? FColor::Green : FColor::Red, false, 5.0f);

	if (!bOverlap || OverlapResults.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("구체 내에 겹친 오브젝트가 없음"));
		return;
	}

	AActor* ClosestInteractableActor = nullptr;
	float MinDistanceSquared = MAX_FLT;
	FVector PlayerLoc = GetActorLocation();

	for (const FOverlapResult& Overlap : OverlapResults)
	{
		AActor* OverlapActor = Overlap.GetActor();
		if (!OverlapActor) continue;

		UE_LOG(LogTemp, Warning, TEXT("현재 확인된 타겟: %s"), *OverlapActor->GetActorNameOrLabel());

		bool bIsValidInteractable = false;

		if (OverlapActor->GetClass()->ImplementsInterface(UInteractionBase::StaticClass()))
		{
			if (IInteractionBase::Execute_CanBeInteracted(OverlapActor))
			{
				bIsValidInteractable = true;
			}
		}
		else if (OverlapActor->ActorHasTag(TEXT("SavePoint")))
		{
			bIsValidInteractable = true;
		}

		if (bIsValidInteractable)
		{
			float DistSquared = FVector::DistSquared(PlayerLoc, OverlapActor->GetActorLocation());
			if (DistSquared < MinDistanceSquared)
			{
				MinDistanceSquared = DistSquared;
				ClosestInteractableActor = OverlapActor;
			}
		}
	}

	if (ClosestInteractableActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("최종 선택된 상호작용 타겟: %s"), *ClosestInteractableActor->GetActorNameOrLabel());

		if (ClosestInteractableActor->GetClass()->ImplementsInterface(UInteractionBase::StaticClass()))
		{
			EInteractionType Type = IInteractionBase::Execute_GetInteractionType(ClosestInteractableActor);

			if (Type == EInteractionType::Door)
			{
				// 방금 조작했던 문인지 확인
				bool bIsSameDoor = (ClosestInteractableActor == LastInteractedDoorActor);

				float CurrentTime = GetWorld()->GetTimeSeconds();

				// 쿨다운 체크 (문이 움직이는 도중 연속 클릭 방지, 약 2.5초~3초)
				if (bIsSameDoor && (CurrentTime - LastDoorInteractTime) < 2.5f)
				{
					return;
				}

				// 최근 상호작용한 문 데이터 갱신
				LastInteractedDoorActor = ClosestInteractableActor;
				LastDoorInteractTime = CurrentTime;

				if (bIsInteractingDoor) return;
				bIsInteractingDoor = true;

				PendingDoorActor = ClosestInteractableActor;
				CurrentPickupLocation = IInteractionBase::Execute_GetInteractionTargetLocation(PendingDoorActor);

				// 충돌 무시
				GetCapsuleComponent()->IgnoreActorWhenMoving(PendingDoorActor, true);

				if (MotionWarpingComp)
				{
					if (bIsSameDoor)
					{
						// 워핑 타겟을 내 현재 위치와 회전값으로 설정
						MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
							TEXT("DoorWarp"), GetActorLocation(), GetActorRotation());
					}
					else // 처음 여는 문 
					{
						// 워핑 거리 계산
						FVector DirectionToPlayer = (GetActorLocation() - CurrentPickupLocation).GetSafeNormal2D();
						FVector TargetWarpLocation = CurrentPickupLocation + (DirectionToPlayer * 65.0f);
						FRotator TargetWarpRotation = PendingDoorActor->GetActorForwardVector().Rotation();

						MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
							TEXT("DoorWarp"), TargetWarpLocation, TargetWarpRotation);
					}
				}

				if (AnimData && AnimData->OpenDoorMontage)
				{
					float AnimDuration = PlayAnimMontage(AnimData->OpenDoorMontage);
					AActor* SafeDoorActor = PendingDoorActor;

					FTimerHandle DoorTimer;
					GetWorldTimerManager().SetTimer(DoorTimer, [this, SafeDoorActor]()
						{
							bIsInteractingDoor = false;
							if (APlayerController* PC = Cast<APlayerController>(GetController()))
							{
								EnableInput(PC);
							}

							// 애니메이션 종료 1.5초 후 콜리전 원상 복구
							if (SafeDoorActor)
							{
								FTimerHandle CollisionRestoreTimer;
								GetWorldTimerManager().SetTimer(CollisionRestoreTimer, FTimerDelegate::CreateLambda([this, SafeDoorActor]()
									{
										if (IsValid(this) && IsValid(SafeDoorActor))
										{
											GetCapsuleComponent()->IgnoreActorWhenMoving(SafeDoorActor, false);
										}
									}), 1.5f, false);
							}
						}, AnimDuration, false);
				}

				IInteractionBase::Execute_OnIneracted(ClosestInteractableActor, this);
			}
			else if (Type == EInteractionType::Ammo || Type == EInteractionType::Heal || Type == EInteractionType::Key)
			{
				if (CurrentInteractingItem)
				{
					ConsumeInteractingItem();
				}

				CurrentInteractingItem = ClosestInteractableActor;
				CurrentPickupLocation = IInteractionBase::Execute_GetInteractionTargetLocation(CurrentInteractingItem);

				FVector LocalItemPos = GetActorTransform().InverseTransformPosition(CurrentPickupLocation);
				UAnimMontage* MontageToPlay = (LocalItemPos.Z < -50.0f) ? AnimData->PickupLowMontage : AnimData->PickupHighMontage;

				if (MontageToPlay)
				{
					PlayAnimMontage(MontageToPlay);
				}
			}
		}
		else if (ClosestInteractableActor->ActorHasTag(TEXT("SavePoint")))
		{
			APlayerController* PC = Cast<APlayerController>(GetController());
			if (PC && PC->GetLocalPlayer())
			{
				if (USaveSubsystem* SaveSub = PC->GetLocalPlayer()->GetSubsystem<USaveSubsystem>())
				{
					SaveSub->ShowSaveUI();
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("구체 내에 일반 물체만 있고, 상호작용 가능한 물체는 없음."));
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
}

float APrototypeCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!StatusComp || StatusComp->IsDead()) return 0.0f;

	// 데미지 적용 (HP 감소 및 사망 판정)
	float ActualDamage = StatusComp->ApplyDamage(DamageAmount);

	// 피격 당할시 화면 흔들림
	if (HitCameraShakeClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->ClientStartCameraShake(HitCameraShakeClass);
		}
	}

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
	if (IsEquipping()) return;

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

	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		AnimInst->StopAllMontages(0.0f);
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
	if (GetIsReloading() || GetIsAiming() || IsEquipping()) return;

	if (StatusComp->CurrHealth >= StatusComp->MaxHealth)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 체력이 가득 참 (HP: %f)"), StatusComp->CurrHealth);
		return;
	}

	if (AnimData && AnimData->HealMontage)
	{
		if (bIsRunning) EndRunning(FInputActionValue());

		PlayAnimMontage(AnimData->HealMontage);
		StatusComp->HealingItemCount--;
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
void APrototypeCharacter::SpawnHealItemVisual()
{
	if (!HealItemClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	CurrHealItem = GetWorld()->SpawnActor<AActor>(HealItemClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (CurrHealItem)
	{
		CurrHealItem->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HealItemSocket"));
		CurrHealItem->SetActorRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));

		AHealItemActor* HealProp = Cast<AHealItemActor>(CurrHealItem);
		if (HealProp)
		{
			if (HealProp->MarkerPillar) HealProp->MarkerPillar->SetHiddenInGame(true);
			if (HealProp->InteractWidget) HealProp->InteractWidget->SetVisibility(false);
		}
	}
}
void APrototypeCharacter::DestroyHealItemVisual()
{
	if (CurrHealItem)
	{
		CurrHealItem->Destroy();
		CurrHealItem = nullptr;
	}
}
void APrototypeCharacter::PopHealItemCap()
{
	if (CurrHealItem)
	{
		AHealItemActor* HealProp = Cast<AHealItemActor>(CurrHealItem);
		if (HealProp && HealProp->CapMesh)
		{
			HealProp->CapMesh->SetHiddenInGame(true);
		}
	}
}

void APrototypeCharacter::PlayPickupAnimation(AActor* TargetItem)
{
}

void APrototypeCharacter::AttachInteractingItem()
{
	if (CurrentInteractingItem)
	{
		CurrentInteractingItem->SetActorEnableCollision(false);

		// 손 소켓에 부착
		CurrentInteractingItem->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("ItemSocket"));

		// 픽업 중 아이템 크기를 0.8배로 줄임
		CurrentInteractingItem->SetActorScale3D(FVector(0.8f));
	}
}

void APrototypeCharacter::ConsumeInteractingItem()
{
	if (CurrentInteractingItem)
	{
		// 여기서 원래의 아이템 습득 로직 실행
		IInteractionBase::Execute_OnIneracted(CurrentInteractingItem, this);
		CurrentInteractingItem = nullptr;
	}
}

#pragma region 인터페이스 구현 
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
#pragma endregion