#include "Character/Components/PlayerCombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Weapon/Weapon.h"
#include "Character/Animation/PlayerAnimInstance.h"
#include "Curves/CurveVector.h"

UPlayerCombatComponent::UPlayerCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작 시 무기 생성
	SpawnDefaultWeapon();
}

void UPlayerCombatComponent::SetupCombat(UCameraComponent* InCamera)
{
	PlayerCamera = InCamera;
}

void UPlayerCombatComponent::SpawnDefaultWeapon()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());

	if (PistolClass)
	{
		PistolWeapon = GetWorld()->SpawnActor<AWeapon>(PistolClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (PistolWeapon)
		{
			PistolWeapon->Equip(Character->GetMesh(), TEXT("WeaponSocket"), Character, Character);
			PistolWeapon->SetActorHiddenInGame(true);
		}
	}

	if (SMGClass)
	{
		SMGWeapon = GetWorld()->SpawnActor<AWeapon>(SMGClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (SMGWeapon)
		{
			SMGWeapon->Equip(Character->GetMesh(), TEXT("BackWeaponSocket"), Character, Character);
			SMGWeapon->SetActorHiddenInGame(false);
		}
	}

	EquippedWeapon = PistolWeapon;
	CurrentWeaponIndex = 1;
	bIsWeaponDrawn = false;
}

void UPlayerCombatComponent::ToggleEquip(UAnimMontage* Montage, UAnimInstance* AnimInst)
{
	if (!EquippedWeapon || bIsAiming) return;

	bIsWeaponDrawn = !bIsWeaponDrawn;

	if (bIsWeaponDrawn)
	{
		EquippedWeapon->SetActorHiddenInGame(false);
	}

	if (AnimInst && Montage)
	{
		AnimInst->Montage_Play(Montage);
	}
}

void UPlayerCombatComponent::Reload()
{
	// 이미 재장전 중이거나 무기가 없으면 리턴
	if (!EquippedWeapon || EquippedWeapon->IsReloading()) return;
	if (EquippedWeapon->IsFullAmmo()) return;

	if (EquippedWeapon->GetReserveAmmo() <= 0) return;

	// 상황에 맞는 몽타주 선택
	UAnimMontage* SelectedMontage = GetSelectedReloadMontage();

	if (!SelectedMontage) return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (AnimInst)
		{
			AnimInst->Montage_Play(SelectedMontage);
			EquippedWeapon->StartReload(); 
		}
	}
}

bool UPlayerCombatComponent::StartAiming()
{
	if (!bIsWeaponDrawn || !EquippedWeapon || EquippedWeapon->IsReloading()) return false;
	bIsAiming = true;

	CurrentSpread = MaxSpread;

	return true;
}

void UPlayerCombatComponent::StopAiming()
{
	if (!bIsAiming) return;
	bIsAiming = false;

	UE_LOG(LogTemp, Error, TEXT("CombatComp: bIsAiming set to FALSE (StopAiming Called)"));
}

void UPlayerCombatComponent::Fire(UAnimMontage* FireMontage, UAnimInstance* AnimInst, TSubclassOf<UCameraShakeBase> CamShake)
{
	if (!bIsAiming || !EquippedWeapon || !PlayerCamera) return;

	if (EquippedWeapon->IsReloading()) return;

	if (!EquippedWeapon->HasAmmo())
	{
		EquippedWeapon->PlayDryFireSound();
		return;
	}

	UAnimMontage* MontageToPlay = (CurrentWeaponIndex == 1) ? Pistol_FireMontage : SMG_FireMontage;

	// 애니메이션 재생
	if (AnimInst && MontageToPlay)
	{
		AnimInst->Montage_Play(MontageToPlay);
	}
	// 카메라 쉐이크 (플레이어가 느끼는 반동)
	if (CamShake && GetWorld())
	{
		UGameplayStatics::PlayWorldCameraShake(GetWorld(), CamShake, PlayerCamera->GetComponentLocation(), 0.0f, 500.0f);
	}

	float RecoilPitch = 0.0f;
	float RecoilYaw = 0.0f;

	if (EquippedWeapon->RecoilCurve)
	{
		FVector CurveValue = EquippedWeapon->RecoilCurve->GetVectorValue(CurrentShotsFired);

		RecoilPitch = CurveValue.Y;
		RecoilYaw = CurveValue.Z;

		// 약간의 랜덤함 추가
		RecoilPitch += FMath::RandRange(-0.5f, 0.5f);
		RecoilYaw += FMath::RandRange(-0.5f, 0.5f);
	}
	else
	{
		RecoilPitch = FMath::RandRange(MaxRecoilPitch, MinRecoilPitch);
		RecoilYaw = FMath::RandRange(MinRecoilYaw, MaxRecoilYaw);
	}

	TargetRecoilRot.Pitch += RecoilPitch;
	TargetRecoilRot.Yaw += RecoilYaw;

	// 다음 총알을 위해 발사 횟수 증가
	CurrentShotsFired++;

	FVector CameraStart = PlayerCamera->GetComponentLocation();
	FVector CameraForward = PlayerCamera->GetForwardVector();

	FVector SpreadDirection = FMath::VRandCone(CameraForward, FMath::DegreesToRadians(CurrentSpread));
	FVector CameraEnd = CameraStart + (SpreadDirection * 10000.f);
	CurrentSpread = FMath::Clamp(CurrentSpread + FireSpreadPenalty, MinSpread, MaxSpread);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(EquippedWeapon);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CameraStart, CameraEnd, ECC_Visibility, Params);
	FVector HitTarget = bHit ? Hit.ImpactPoint : CameraEnd;

	EquippedWeapon->Fire(HitTarget);
}

void UPlayerCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsAiming)
	{
		UpdateHandIK();
		CalculateAimOffset();

		UpdateSpread(DeltaTime);
	}
	else
	{
		CurrentSpread = MaxSpread;
	}

	HandleRecoil(DeltaTime);
}

void UPlayerCombatComponent::UpdateHandIK()
{
	// IK 타겟을 무기의 레이저 끝점 등으로 설정
	if (EquippedWeapon && bIsWeaponDrawn)
	{
		HandIKTargetLocation = EquippedWeapon->GetLaserTargetLocation();
	}
}

void UPlayerCombatComponent::CalculateAimOffset()
{
	if (!bIsAiming || !PlayerCamera) return;

	FVector CameraForward = PlayerCamera->GetForwardVector();
	FRotator CharacterRotation = GetOwner()->GetActorRotation();

	FRotator DeltaRotation = (CameraForward.Rotation() - CharacterRotation);
	DeltaRotation.Normalize();

	AimYaw = DeltaRotation.Yaw;
	AimPitch = DeltaRotation.Pitch;

	AimYaw = FMath::Clamp(AimYaw, -90.0f, 90.0f);
	AimPitch = FMath::Clamp(AimPitch, -90.0f, 90.0f);
}

void UPlayerCombatComponent::HandleRecoil(float DeltaTime)
{
	CurrentRecoilRot = FMath::RInterpTo(CurrentRecoilRot, TargetRecoilRot, DeltaTime, RecoilInterpSpeed);

	FRotator DeltaRot = CurrentRecoilRot - LastRecoilRot;
	LastRecoilRot = CurrentRecoilRot;

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (AController* Controller = Pawn->GetController())
		{
			FRotator NewControlRot = Controller->GetControlRotation() + DeltaRot;
			Controller->SetControlRotation(NewControlRot);
		}
	}

	if (TargetRecoilRot.IsNearlyZero(0.01f))
	{
		TargetRecoilRot = FRotator::ZeroRotator;
		CurrentRecoilRot = FRotator::ZeroRotator;
		LastRecoilRot = FRotator::ZeroRotator;
	}
	else
	{
		TargetRecoilRot = FMath::RInterpTo(TargetRecoilRot, FRotator::ZeroRotator, DeltaTime, RecoilRecoverySpeed);
	}
}

UAnimMontage* UPlayerCombatComponent::GetCurrentEquipMontage(bool bEquip)
{
	// 현재 인덱스에 따라 다른 몽타주 반환
	switch (CurrentWeaponIndex)
	{
	case 1: return bEquip ? Pistol_EquipMontage : Pistol_UnEquipMontage;
	case 2: return bEquip ? SMG_EquipMontage : SMG_UnEquipMontage;
	case 3: return bEquip ? Melee_EquipMontage : Melee_UnEquipMontage;
	default: return nullptr;
	}
}

UAnimMontage* UPlayerCombatComponent::GetSelectedReloadMontage()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return nullptr;

	// Pistol
	if (CurrentWeaponIndex == 1)
	{
		return Pistol_ReloadMontage;
	}

	// SMG
	if (CurrentWeaponIndex == 2)
	{
		bool bIsCrouched = OwnerChar->bIsCrouched; 
		bool bIsAimingNow = bIsAiming;

		if (bIsCrouched)
		{
			return bIsAimingNow ? SMG_ReloadSet.CrouchAimReload : SMG_ReloadSet.CrouchReload;
		}
		else
		{
			return bIsAimingNow ? SMG_ReloadSet.StandingAimReload : SMG_ReloadSet.StandingReload;
		}
	}

	return nullptr;
}

void UPlayerCombatComponent::HandleWeaponAttachment(bool bToHand)
{
	if (!EquippedWeapon) return;
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return;

	if (bToHand) // 장착할 때 (Equip)
	{
		FName HandSocket = (CurrentWeaponIndex == 1) ? TEXT("WeaponSocket") : TEXT("SMG_Socket");
		EquippedWeapon->AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocket);
		EquippedWeapon->SetActorHiddenInGame(false);
	}
	else // 해제할 때 (UnEquip)
	{
		if (CurrentWeaponIndex == 2) // SMG
		{
			EquippedWeapon->AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("BackWeaponSocket"));
			EquippedWeapon->SetActorHiddenInGame(false); // 등에서는 보이게 
		}
		else // 권총
		{
			EquippedWeapon->AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HolsterSocket"));
			EquippedWeapon->SetActorHiddenInGame(true);
		}
	}
}

bool UPlayerCombatComponent::GetIsReloading() const
{
	if (EquippedWeapon && EquippedWeapon->IsReloading())
	{
		return true;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (AnimInst)
		{
			bool bIsSMGReloading =
				AnimInst->Montage_IsPlaying(SMG_ReloadSet.StandingReload) ||
				AnimInst->Montage_IsPlaying(SMG_ReloadSet.StandingAimReload) ||
				AnimInst->Montage_IsPlaying(SMG_ReloadSet.CrouchReload) ||
				AnimInst->Montage_IsPlaying(SMG_ReloadSet.CrouchAimReload);

			return AnimInst->Montage_IsPlaying(Pistol_ReloadMontage) || bIsSMGReloading;
		}
	}
	return false;
}

void UPlayerCombatComponent::ChangeWeapon(int32 NewWeaponIndex, UAnimInstance* AnimInst)
{
	if (bIsAiming || CurrentWeaponIndex == NewWeaponIndex) return;

	// 현재 들고 있던 무기를 집어넣는 처리
	if (EquippedWeapon)
	{
		EquippedWeapon->SetIsReloading(false);
		if (AnimInst) AnimInst->Montage_Stop(0.2f);

		// 기존 무기가 MP5 = 등으로, 권총 = 숨기기 
		if (CurrentWeaponIndex == 2)
		{
			HandleWeaponAttachment(false); // 등으로 보냄
		}
		else
		{
			EquippedWeapon->SetActorHiddenInGame(true); // 권총은 숨김
		}
	}

	// 새 무기로 변경
	CurrentWeaponIndex = NewWeaponIndex;
	if (CurrentWeaponIndex == 1) EquippedWeapon = PistolWeapon;
	else if (CurrentWeaponIndex == 2) EquippedWeapon = SMGWeapon;

	// 새 무기가 SMG라면 무조건 보이도록. 
	if (CurrentWeaponIndex == 2 && EquippedWeapon)
	{
		EquippedWeapon->SetActorHiddenInGame(false);
	}

	// ABP 동기화 
	if (AnimInst)
	{
		UPlayerAnimInstance* MyAnimInst = Cast<UPlayerAnimInstance>(AnimInst);
		if (MyAnimInst && EquippedWeapon) MyAnimInst->WeaponMesh = EquippedWeapon->WeaponMesh;
	}
}

void UPlayerCombatComponent::StartFire(UAnimMontage* InFireMontage, UAnimInstance* InAnimInst, TSubclassOf<UCameraShakeBase> InCamShake)
{
	if (!bIsWeaponDrawn || !EquippedWeapon) return;

	bIsFiring = true;
	CurrentShotsFired = 0;

	CachedFireMontage = InFireMontage;
	CachedAnimInst = InAnimInst;
	CachedCamShake = InCamShake;

	if (EquippedWeapon->IsAutomatic())
	{
		GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &UPlayerCombatComponent::AutoFireLogic, EquippedWeapon->GetFireRate(), true, 0.0f);
	}
	else
	{
		AutoFireLogic();
	}
}

void UPlayerCombatComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(FireTimer);

	bIsFiring = false;
	CurrentShotsFired = 0;
}

void UPlayerCombatComponent::AutoFireLogic()
{
	if (!EquippedWeapon || !bIsAiming || EquippedWeapon->IsReloading())
	{
		StopFire();
		return;
	}

	Fire(CachedFireMontage, CachedAnimInst, CachedCamShake);

	if (!EquippedWeapon->HasAmmo())
	{
		StopFire();
	}
}

void UPlayerCombatComponent::UpdateSpread(float DeltaTime)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	float Speed = OwnerCharacter->GetVelocity().Size();

	if (Speed > 10.0f)
	{
		CurrentSpread = FMath::FInterpTo(CurrentSpread, MaxSpread, DeltaTime, SpreadExpandRate);
	}
	else
	{
		CurrentSpread = FMath::FInterpTo(CurrentSpread, MinSpread, DeltaTime, SpreadShrinkRate);
	}
}