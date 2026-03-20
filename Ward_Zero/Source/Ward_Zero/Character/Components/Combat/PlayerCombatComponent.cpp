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
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Projectile/Projectile.h"
#include "Weapon/Data/ProjectileData.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Character/Noise/NoiseFucLibrary/PlayerNoise.h"

UPlayerCombatComponent::UPlayerCombatComponent() { PrimaryComponentTick.bCanEverTick = true; }

void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedOwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!CachedOwnerCharacter) return;

	InitializeProjectilePool();

	// CombatData 에셋 참조 및 수치 동기화
	if (APrototypeCharacter* OwnerChar = Cast<APrototypeCharacter>(GetOwner()))
	{
		if (OwnerChar->CombatData)
		{
			this->CombatData = OwnerChar->CombatData;

			RecoilInterpSpeed = CombatData->RecoilInterpSpeed;
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

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime < EquippedWeapon->GetFireRate())
	{
		return;
	}
	LastFireTime = CurrentTime;

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

	if (!EquippedWeapon->HasAmmo() || EquippedWeapon->IsReloading())
	{
		EquippedWeapon->PlayDryFireSound();
		return;
	}

	PlayFireEffects(FireMontage, AnimInst, CamShake);
	CalculateShotRecoil();

	if (CamShake) UGameplayStatics::PlayWorldCameraShake(GetWorld(), CamShake, PlayerCamera->GetComponentLocation(), 0.f, 500.f);

	// 라인트레이스 (발사 시점에 딱 한 번만 수행하여 목표 지점 확정)
	// 라인 트레이스 수행
	FVector Start = PlayerCamera->GetComponentLocation();
	FVector Dir = FMath::VRandCone(PlayerCamera->GetForwardVector(), FMath::DegreesToRadians(CurrentSpread * 0.5f));
	FVector End = Start + (Dir * 10000.f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(EquippedWeapon);
	Params.bReturnPhysicalMaterial = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel2, Params);

	// 총알이 최종적으로 도달해야 할 목표 지점 총알 투사체 방식 
	FVector FinalHitTarget = bHit ? Hit.ImpactPoint : End;

	// 히트 결과 처리 및 궤적 생성 - 트레이서 방식 Test 
	if (bHit)
	{
		ProcessHit(Hit, Dir);
	}
	SpawnTracer(EquippedWeapon->WeaponMesh->GetSocketLocation(TEXT("Muzzle")), FinalHitTarget);

	if (Hit.PhysMaterial.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Hit Physical Material Found: %s"), *Hit.PhysMaterial->GetName());
	}

	// 오브젝트 풀에서 총알 활성화 (메모리 할당 0) - 총알 투사체 방식 Test
	/*AProjectile* Proj = GetProjectileFromPool();
	if (Proj)
	{
		FVector MuzzleLoc = EquippedWeapon->WeaponMesh->GetSocketLocation(TEXT("Muzzle"));
		FRotator FireRot = UKismetMathLibrary::FindLookAtRotation(MuzzleLoc, FinalHitTarget);

		Proj->SetActorLocationAndRotation(MuzzleLoc, FireRot);
		Proj->InitializeProjectile(EquippedWeapon->WeaponData->ProjectileData);
		Proj->SetProjectileActive(true);
	}*/

	// 무기 데이터 업데이트
	EquippedWeapon->SpendRound();
	CurrentShotsFired++;
	CurrentSpread = FMath::Clamp(CurrentSpread + FireSpreadPenalty, 0.0f, MaxSpread);
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
}

void UPlayerCombatComponent::UpdateSpread(float DeltaTime)
{
	if (!CachedOwnerCharacter) return;

	float Speed = CachedOwnerCharacter->GetVelocity().Size();
	if (Speed > 10.0f) CurrentSpread = FMath::FInterpTo(CurrentSpread, MaxSpread, DeltaTime, SpreadExpandRate);
	else CurrentSpread = FMath::FInterpTo(CurrentSpread, MinSpread, DeltaTime, SpreadShrinkRate);
}

void UPlayerCombatComponent::InitializeProjectilePool()
{
	if (!ProjectileClass) return;

	for (int32 i = 0; i < ProjectilePoolSize; i++)
	{
		FActorSpawnParameters Params;
		Params.Owner = GetOwner();
		Params.Instigator = Cast<APawn>(GetOwner());

		AProjectile* NewProj = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
		if (NewProj)
		{
			NewProj->SetProjectileActive(false); // 비활성 상태로 풀에 보관
			ProjectilePool.Add(NewProj);
		}
	}
}

AProjectile* UPlayerCombatComponent::GetProjectileFromPool()
{
	// 풀이 비어있는지 먼저 확인
	if (ProjectilePool.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Projectile Pool is EMPTY! Check ProjectileClass in Blueprint."));
		return nullptr;
	}

	// 비활성화된 총알 찾기
	for (AProjectile* Proj : ProjectilePool)
	{
		if (IsValid(Proj) && !Proj->IsProjectileActive())
		{
			return Proj;
		}
	}

	// 모든 총알이 사용 중일 때 안전하게 첫 번째 요소 반환 
	if (ProjectilePool.IsValidIndex(0))
	{
		return ProjectilePool[0];
	}

	return nullptr;
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

UAnimMontage* UPlayerCombatComponent::GetSelectedFireMontage() const
{
	if (!CachedOwnerCharacter) return nullptr;

	bool bIsCrouched = CachedOwnerCharacter->bIsCrouched;

	if (CurrentWeaponIndex == 1) // Pistol
	{
		return bIsCrouched ? Pistol_CrouchFireMontage : Pistol_FireMontage;
	}
	else if (CurrentWeaponIndex == 2) // SMG
	{
		return bIsCrouched ? SMG_CrouchFireMontage : SMG_FireMontage;
	}

	return nullptr;
}

void UPlayerCombatComponent::PlayFireEffects(UAnimMontage* FireMontage, UAnimInstance* AnimInst, TSubclassOf<UCameraShakeBase> CamShake)
{
	UAnimMontage* MontageToPlay = FireMontage ? FireMontage : GetSelectedFireMontage();
	if (AnimInst && MontageToPlay)
	{
		AnimInst->Montage_Play(MontageToPlay);
	}

	// 카메라 쉐이크 및 무기 이펙트 재생
	if (CamShake)
	{
		UGameplayStatics::PlayWorldCameraShake(GetWorld(), CamShake, PlayerCamera->GetComponentLocation(), 0.f, 500.f);
	}

	if (EquippedWeapon)
	{
		EquippedWeapon->FireEffectsOnly();
	}
}

void UPlayerCombatComponent::CalculateShotRecoil()
{
	float RecoilPitch = 0.0f;
	float RecoilYaw = 0.0f;
	float Intensity = EquippedWeapon->GetRecoilIntensity();

	if (EquippedWeapon->RecoilCurve)
	{
		FVector CurveVal = EquippedWeapon->RecoilCurve->GetVectorValue((float)CurrentShotsFired);
		RecoilPitch = CurveVal.Y * Intensity;
		RecoilYaw = CurveVal.X * Intensity;

		/*RecoilPitch = FMath::Max(0.0f, RecoilPitch + FMath::RandRange(-0.5f, 0.5f));
		RecoilYaw += FMath::RandRange(-0.5f, 0.5f);*/
	}
	else if (EquippedWeapon->WeaponData)
	{
		float RandomVal = EquippedWeapon->WeaponData->HorizontalRecoilRandomness;
		RecoilYaw += FMath::RandRange(-RandomVal, RandomVal);

		RecoilPitch = FMath::Max(0.0f, Intensity + FMath::RandRange(-RandomVal * 0.2f, RandomVal * 0.2f));
	}

	TargetRecoilRot.Pitch += RecoilPitch;
	TargetRecoilRot.Yaw += RecoilYaw;
}

void UPlayerCombatComponent::ProcessHit(const FHitResult& Hit, const FVector& ShotDir)
{
	UProjectileData* ProjData = EquippedWeapon->WeaponData ? EquippedWeapon->WeaponData->ProjectileData : nullptr;
	if (!ProjData) return;

	EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);

	// VFX
	UNiagaraSystem* Effect = ProjData->ImpactEffectMap.Contains(SurfaceType) ? ProjData->ImpactEffectMap[SurfaceType] : ProjData->ImpactEffectMap.FindRef(SurfaceType_Default);
	if (Effect) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Effect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());

	// SFX
	USoundBase* Sound = ProjData->ImpactSoundMap.Contains(SurfaceType) ? ProjData->ImpactSoundMap[SurfaceType] : ProjData->ImpactSoundMap.FindRef(SurfaceType_Default);
	if (Sound) UGameplayStatics::PlaySoundAtLocation(this, Sound, Hit.ImpactPoint);

	// Damage
	UGameplayStatics::ApplyPointDamage(Hit.GetActor(), ProjData->Damage, ShotDir, Hit, Cast<APawn>(GetOwner())->GetController(), GetOwner(), ProjData->DamageTypeClass);

	// AI Noise
	UPlayerNoise::ReportNoise(GetWorld(), Cast<APawn>(GetOwner()), Hit.ImpactPoint, ProjData->ImpactNoiseLoudness, ProjData->ImpactNoiseRange, ProjData->ImpactNoiseTag);

	DrawDebugSphere(GetWorld(),Hit.ImpactPoint,10.0f,12,FColor::Red,false,2.0f,0,1.5f);
}

void UPlayerCombatComponent::SpawnTracer(const FVector& Start, const FVector& End)
{
	{
		// Attached를 사용하여 총구에 고정
		UNiagaraComponent* Tracer = UNiagaraFunctionLibrary::SpawnSystemAttached(
			EquippedWeapon->WeaponData->ProjectileData->TracerEffect,
			EquippedWeapon->WeaponMesh,
			TEXT("Muzzle"),              
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);

		if (Tracer)
		{
			// 나이아가라 시스템 내부의 변수(TracerEnd)에 목적지 좌표 전달
			Tracer->SetVariableVec3(TEXT("TracerEnd"), End);
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

	HandleWeaponAttachment(true);

	if (EquippedWeapon)
	{
		EquippedWeapon->EnableLaserSight(true);
	}
	return true;
}

void UPlayerCombatComponent::StopAiming() 
{ 
	bIsAiming = false; 

	HandleWeaponAttachment(true);

	if (EquippedWeapon)
	{
		EquippedWeapon->EnableLaserSight(false);
	}
}

void UPlayerCombatComponent::CalculateAimOffset()
{
	if (!CachedOwnerCharacter || !PlayerCamera) return;

	FRotator ControlRot = CachedOwnerCharacter->GetControlRotation();
	FRotator ActorRot = CachedOwnerCharacter->GetActorRotation();

	// 두 회전의 차이 계산
	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot);
	AimYaw = 0.0f;
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
		FName TargetSocketName;

		if (CurrentWeaponIndex == 1) // Pistol
		{
			if (bIsAiming)
			{
				TargetSocketName = OwnerChar->bIsCrouched ? TEXT("WeaponSocket_Crouch") : TEXT("WeaponSocket");
			}
			else
			{
				TargetSocketName = TEXT("PistolRelaxSocket");
			}
		}
		else // SMG
		{
			TargetSocketName = TEXT("SMG_Socket");
		}

		EquippedWeapon->AttachToComponent(OwnerChar->GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocketName);

		if (EquippedWeapon->WeaponMesh)
		{
			EquippedWeapon->WeaponMesh->SetRelativeRotation(FRotator::ZeroRotator);
		}
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