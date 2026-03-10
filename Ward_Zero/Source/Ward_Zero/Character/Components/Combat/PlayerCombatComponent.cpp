#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Weapon.h"
#include "Character/Animation/PlayerAnimInstance.h"
#include "Curves/CurveVector.h"
#include "Character/Data/Combat/CharacterCombatData.h"
#include "Character/Data/Camera/CameraData.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Weapon/Data/WeaponData.h"

UPlayerCombatComponent::UPlayerCombatComponent() { PrimaryComponentTick.bCanEverTick = true; }

void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// CombatData 에셋 참조 및 수치 동기화
	if (APrototypeCharacter* OwnerChar = Cast<APrototypeCharacter>(GetOwner()))
	{
		if (OwnerChar->CombatData)
		{
			this->CombatData = OwnerChar->CombatData;

			RecoilInterpSpeed = CombatData->RecoilInterpSpeed;
			RecoilRecoverySpeed = CombatData->RecoilRecoverySpeed;
			MaxSpread = CombatData->MaxSpread;
			FireSpreadPenalty = CombatData->FireSpreadPenalty;
		}
	}
	SpawnDefaultWeapon();
}

void UPlayerCombatComponent::SetupCombat(UCameraComponent* InCamera) { PlayerCamera = InCamera; }

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
			PistolWeapon->SetActorEnableCollision(false);
			if (PistolWeapon->WeaponMesh)
			{
				PistolWeapon->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}

	if (SMGClass)
	{
		SMGWeapon = GetWorld()->SpawnActor<AWeapon>(SMGClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (SMGWeapon)
		{
			SMGWeapon->Equip(Character->GetMesh(), TEXT("BackWeaponSocket"), Character, Character);
			SMGWeapon->SetActorHiddenInGame(false);
			SMGWeapon->SetActorEnableCollision(false);
			if (SMGWeapon->WeaponMesh)
			{
				SMGWeapon->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}

	EquippedWeapon = PistolWeapon;
	CurrentWeaponIndex = 1;
	bIsWeaponDrawn = false;
}


void UPlayerCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CalculateAimOffset();

	if (bIsAiming)
	{
		UpdateHandIK();
		UpdateSpread(DeltaTime);
	}
	else
	{
		CurrentSpread = MaxSpread;
	}

	HandleRecoil(DeltaTime);
}

void UPlayerCombatComponent::StartFire(UAnimMontage* InFireMontage, UAnimInstance* InAnimInst, TSubclassOf<UCameraShakeBase> InCamShake)
{
	if (!bIsWeaponDrawn || !EquippedWeapon || EquippedWeapon->IsReloading() || !bIsAiming) return;

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

void UPlayerCombatComponent::AutoFireLogic()
{
	// 재장전 중이거나 조준이 풀리면 연사 중단
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

void UPlayerCombatComponent::Fire(UAnimMontage* FireMontage, UAnimInstance* AnimInst, TSubclassOf<UCameraShakeBase> CamShake)
{
	if (!EquippedWeapon || !PlayerCamera) return;

	// 애니메이션 재생
	UAnimMontage* MontageToPlay = FireMontage ? FireMontage : ((CurrentWeaponIndex == 1) ? Pistol_FireMontage : SMG_FireMontage);
	if (AnimInst && MontageToPlay) AnimInst->Montage_Play(MontageToPlay);

	// 카메라 쉐이크
	if (CamShake) UGameplayStatics::PlayWorldCameraShake(GetWorld(), CamShake, PlayerCamera->GetComponentLocation(), 0.f, 500.f);

	float RecoilPitch = 0.0f;
	float RecoilYaw = 0.0f;
	float Intensity = EquippedWeapon->GetRecoilIntensity();

	if (EquippedWeapon->RecoilCurve)
	{
		FVector CurveVal = EquippedWeapon->RecoilCurve->GetVectorValue((float)CurrentShotsFired);
		RecoilPitch = CurveVal.Y * Intensity;
		RecoilYaw = CurveVal.X * Intensity;
		RecoilPitch += FMath::RandRange(-0.5f, 0.5f);
		RecoilYaw += FMath::RandRange(-0.5f, 0.5f);
	}
	else if (EquippedWeapon->WeaponData)
	{
		float RandomVal = EquippedWeapon->WeaponData->HorizontalRecoilRandomness;
		RecoilYaw += FMath::RandRange(-RandomVal, RandomVal);
		RecoilPitch += FMath::RandRange(-RandomVal * 0.2f, RandomVal * 0.2f);
	}

	TargetRecoilRot.Pitch = FMath::Clamp(TargetRecoilRot.Pitch + RecoilPitch, -30.0f, 15.0f);
	TargetRecoilRot.Yaw = FMath::Clamp(TargetRecoilRot.Yaw + RecoilYaw, -10.0f, 10.0f);

	// [원본 복구] 탄퍼짐 로직
	FVector Start = PlayerCamera->GetComponentLocation();
	FVector Dir = FMath::VRandCone(PlayerCamera->GetForwardVector(), FMath::DegreesToRadians(CurrentSpread));

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(EquippedWeapon);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + (Dir * 10000.f), ECC_GameTraceChannel2, Params);
	EquippedWeapon->Fire(bHit ? Hit.ImpactPoint : Hit.TraceEnd);

	CurrentShotsFired++;
	CurrentSpread = FMath::Clamp(CurrentSpread + 2.5f, 0.0f, 5.0f); // FireSpreadPenalty, MaxSpread
}

void UPlayerCombatComponent::HandleRecoil(float DeltaTime)
{
	CurrentRecoilRot = FMath::RInterpTo(CurrentRecoilRot, TargetRecoilRot, DeltaTime, RecoilInterpSpeed);
	FRotator Delta = CurrentRecoilRot - LastRecoilRot;
	LastRecoilRot = CurrentRecoilRot;

	if (APawn* P = Cast<APawn>(GetOwner()))
	{
		if (auto* PC = P->GetController())
		{
			PC->SetControlRotation(PC->GetControlRotation() + Delta);
		}
	}

	// 반동 회복 및 변수 완전 초기화
	if (TargetRecoilRot.IsNearlyZero(0.01f))
	{
		TargetRecoilRot = FRotator::ZeroRotator;
		CurrentRecoilRot = FRotator::ZeroRotator;
		LastRecoilRot = FRotator::ZeroRotator;
	}
	else
	{
		float RecoverySpeed = EquippedWeapon ? EquippedWeapon->GetRecoilRecoverySpeed() : RecoilRecoverySpeed;
		TargetRecoilRot = FMath::RInterpTo(TargetRecoilRot, FRotator::ZeroRotator, DeltaTime, RecoverySpeed);
	}
}

void UPlayerCombatComponent::UpdateSpread(float DeltaTime)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	float Speed = OwnerCharacter->GetVelocity().Size();
	if (Speed > 10.0f) CurrentSpread = FMath::FInterpTo(CurrentSpread, MaxSpread, DeltaTime, SpreadExpandRate);
	else CurrentSpread = FMath::FInterpTo(CurrentSpread, MinSpread, DeltaTime, SpreadShrinkRate);
}

void UPlayerCombatComponent::SetCameraPitchLimit(bool IsAiming)
{
	if (!GetOwner()) return;

	APrototypeCharacter* OwnerChar = Cast<APrototypeCharacter>(GetOwner());
	
	if (!OwnerChar || !OwnerChar->CameraConfig) return;

	if (APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			auto* Config = OwnerChar->CameraConfig;
			if (bIsAiming)
			{
				PC->PlayerCameraManager->ViewPitchMin = Config->AimPitchMin;
				PC->PlayerCameraManager->ViewPitchMax = Config->AimPitchMax;
			}
			else
			{
				PC->PlayerCameraManager->ViewPitchMin = Config->DefaultPitchMin;
				PC->PlayerCameraManager->ViewPitchMax = Config->DefaultPitchMax;
			}
		}
	}
}

void UPlayerCombatComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(FireTimer);
	bIsFiring = false;
	CurrentShotsFired = 0; // 발사 중단 시 샷 카운트 초기화
}

bool UPlayerCombatComponent::StartAiming()
{
	if (!bIsWeaponDrawn || GetIsReloading()) return false;
	bIsAiming = true;
	CurrentSpread = MaxSpread;
	return true;
}

void UPlayerCombatComponent::StopAiming() { bIsAiming = false; }

void UPlayerCombatComponent::CalculateAimOffset()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner || !PlayerCamera) return;
	FRotator Delta = (PlayerCamera->GetForwardVector().Rotation() - Owner->GetActorRotation()).GetNormalized();
	AimYaw = FMath::Clamp(Delta.Yaw, -90.f, 90.f);
	AimPitch = FMath::Clamp(Delta.Pitch, -90.f, 90.f);
}

void UPlayerCombatComponent::UpdateHandIK() { if (EquippedWeapon) HandIKTargetLocation = EquippedWeapon->GetLaserTargetLocation(); }

bool UPlayerCombatComponent::GetIsReloading() const { return EquippedWeapon && EquippedWeapon->IsReloading(); }

void UPlayerCombatComponent::ToggleEquip(UAnimMontage* Montage, UAnimInstance* AnimInst)
{
	if (!EquippedWeapon) return;
	bIsWeaponDrawn = !bIsWeaponDrawn;

	USoundBase* SoundToPlay = bIsWeaponDrawn ? EquippedWeapon->WeaponData->EquipSound : EquippedWeapon->WeaponData->UnequipSound;
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, GetOwner()->GetActorLocation());
	}

	UAnimMontage* MontageToPlay = Montage ? Montage : GetCurrentEquipMontage(bIsWeaponDrawn);
	if (bIsWeaponDrawn) EquippedWeapon->SetActorHiddenInGame(false);
	if (AnimInst && MontageToPlay) AnimInst->Montage_Play(MontageToPlay);
}

void UPlayerCombatComponent::ChangeWeapon(int32 NewIndex, UAnimInstance* AnimInst)
{
	if (CurrentWeaponIndex == NewIndex && bIsWeaponDrawn) return;

	StopAiming();
	StopFire();

	// 기존 무기 숨기기 로직
	if (EquippedWeapon)
	{
		EquippedWeapon->SetIsReloading(false);
		if (CurrentWeaponIndex == 2) HandleWeaponAttachment(false); // SMG는 등으로
		else EquippedWeapon->SetActorHiddenInGame(true); // 권총은 숨김
	}

	if (EquippedWeapon && EquippedWeapon->WeaponData->UnequipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->WeaponData->UnequipSound, GetOwner()->GetActorLocation());
	}

	CurrentWeaponIndex = NewIndex;
	EquippedWeapon = (CurrentWeaponIndex == 1) ? PistolWeapon : SMGWeapon;

	// 새 무기 꺼내기 설정
	bIsWeaponDrawn = true;
	if (EquippedWeapon)
	{
		EquippedWeapon->SetActorHiddenInGame(false);
		HandleWeaponAttachment(true); // 손으로 부착
	}

	if (EquippedWeapon && EquippedWeapon->WeaponData->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->WeaponData->EquipSound, GetOwner()->GetActorLocation());
	}

	// 애니메이션 인스턴스에 메쉬 정보 업데이트
	if (AnimInst)
	{
		UPlayerAnimInstance* MyAnimInst = Cast<UPlayerAnimInstance>(AnimInst);
		if (MyAnimInst && EquippedWeapon) MyAnimInst->WeaponMesh = EquippedWeapon->WeaponMesh;

		// 장착 몽타주 재생
		UAnimMontage* EquipMontage = GetCurrentEquipMontage(true);
		if (EquipMontage) AnimInst->Montage_Play(EquipMontage);
	}
}

void UPlayerCombatComponent::Reload()
{
	if (!bIsWeaponDrawn || !EquippedWeapon || EquippedWeapon->IsReloading() || EquippedWeapon->IsFullAmmo()) return;
	if (EquippedWeapon->GetReserveAmmo() <= 0) { EquippedWeapon->PlayDryFireSound(); return; }

	UAnimMontage* Selected = GetSelectedReloadMontage();
	UAnimInstance* AnimInst = Cast<ACharacter>(GetOwner())->GetMesh()->GetAnimInstance();

	if (AnimInst && Selected) {
		AnimInst->Montage_Play(Selected);
		EquippedWeapon->StartReload();
	}
}

UAnimMontage* UPlayerCombatComponent::GetSelectedReloadMontage()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return nullptr;
	if (CurrentWeaponIndex == 1) return Pistol_ReloadMontage;
	if (CurrentWeaponIndex == 2)
	{
		bool bIsCrouched = OwnerChar->bIsCrouched;
		if (bIsCrouched) return bIsAiming ? SMG_ReloadSet.CrouchAimReload : SMG_ReloadSet.CrouchReload;
		else return bIsAiming ? SMG_ReloadSet.StandingAimReload : SMG_ReloadSet.StandingReload;
	}
	return nullptr;
}

UAnimMontage* UPlayerCombatComponent::GetCurrentEquipMontage(bool bEquip)
{
	switch (CurrentWeaponIndex)
	{
	case 1: return bEquip ? Pistol_EquipMontage : Pistol_UnEquipMontage;
	case 2: return bEquip ? SMG_EquipMontage : SMG_UnEquipMontage;
	default: return nullptr;
	}
}

void UPlayerCombatComponent::HandleWeaponAttachment(bool bToHand)
{
	if (!EquippedWeapon) return;
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return;

	if (bToHand)
	{
		FName HandSocket = (CurrentWeaponIndex == 1) ? TEXT("WeaponSocket") : TEXT("SMG_Socket");
		EquippedWeapon->AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocket);
		EquippedWeapon->SetActorHiddenInGame(false);
	}
	else
	{
		if (CurrentWeaponIndex == 2)
		{
			EquippedWeapon->AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("BackWeaponSocket"));
			EquippedWeapon->SetActorHiddenInGame(false);
		}
		else
		{
			EquippedWeapon->AttachToComponent(OwnerChar->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HolsterSocket"));
			EquippedWeapon->SetActorHiddenInGame(true);
		}
	}
}

void UPlayerCombatComponent::CancelReload(UAnimInstance* AnimInst)
{
	if (AnimInst)
	{
		AnimInst->Montage_Stop(0.2f, SMG_ReloadSet.CrouchReload);
		AnimInst->Montage_Stop(0.2f, SMG_ReloadSet.CrouchAimReload);
		AnimInst->Montage_Stop(0.2f, Pistol_ReloadMontage);
	}
	if (EquippedWeapon)
	{
		EquippedWeapon->SetIsReloading(false);
		EquippedWeapon->ShowMagazine();
	}
}