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
			SMGWeapon->Equip(Character->GetMesh(), TEXT("WeaponSocket"), Character, Character);
			SMGWeapon->SetActorHiddenInGame(true);
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
	
	UPlayerAnimInstance* MyAnimInst = Cast<UPlayerAnimInstance>(AnimInst);
	if (MyAnimInst)
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon Mesh Success"));
		MyAnimInst->WeaponMesh = EquippedWeapon->WeaponMesh;
	}

	// 2. 애니메이션 재생
	if (AnimInst && Montage)
	{
		AnimInst->Montage_Play(Montage);
	}
}

void UPlayerCombatComponent::Reload()
{
	if (!EquippedWeapon || EquippedWeapon->IsReloading()) return;

	if (EquippedWeapon->IsFullAmmo()) return;

	UE_LOG(LogTemp, Warning, TEXT("Reload System Active"));

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();

		if (AnimInst && ReloadMontage)
		{
			AnimInst->Montage_Play(ReloadMontage);

			EquippedWeapon->StartReload();
		}
	}
}

bool UPlayerCombatComponent::StartAiming()
{
	if (!bIsWeaponDrawn || !EquippedWeapon || EquippedWeapon->IsReloading()) return false;
	bIsAiming = true;
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

	// 애니메이션 재생 (플레이어의 행동)
	if (AnimInst && FireMontage)
	{
		AnimInst->Montage_Play(FireMontage);
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
	FVector CameraEnd = CameraStart + (PlayerCamera->GetForwardVector() * 10000.f);

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

bool UPlayerCombatComponent::GetIsReloading() const
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && ReloadMontage)
	{
		UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (AnimInst)
		{
			return AnimInst->Montage_IsPlaying(ReloadMontage);
		}
	}
	return false;
}

void UPlayerCombatComponent::ChangeWeapon(int32 NewWeaponIndex, UAnimInstance* AnimInst)
{
	if (bIsAiming || CurrentWeaponIndex == NewWeaponIndex) return;

	if (bIsWeaponDrawn && EquippedWeapon)
	{
		EquippedWeapon->SetActorHiddenInGame(true);
	}

	CurrentWeaponIndex = NewWeaponIndex;
	if (CurrentWeaponIndex == 1) EquippedWeapon = PistolWeapon;
	else if (CurrentWeaponIndex == 2) EquippedWeapon = SMGWeapon;

	// 무기를 꺼낸 상태(Q)였다면, 새로 바꾼 무기를 즉시 보여줌
	if (bIsWeaponDrawn && EquippedWeapon)
	{
		EquippedWeapon->SetActorHiddenInGame(false);

		UPlayerAnimInstance* MyAnimInst = Cast<UPlayerAnimInstance>(AnimInst);
		if (MyAnimInst) MyAnimInst->WeaponMesh = EquippedWeapon->WeaponMesh;
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