#include "Character/Components/PlayerCombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Weapon/Weapon.h"
#include "Character/Animation/PlayerAnimInstance.h"

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
	// 1. 무기 클래스가 설정되어 있는지 확인
	if (DefaultWeaponClass)
	{
		// 2. 무기 액터 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());

		AWeapon* NewWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (NewWeapon)
		{
			EquippedWeapon = NewWeapon;

			// 3. 캐릭터 메쉬의 소켓에 무기 부착
			ACharacter* Character = Cast<ACharacter>(GetOwner());
			if (Character)
			{
				// Weapon 클래스의 Equip 함수 호출 (여기서 Attach 및 Owner 설정 등이 처리됨)
				EquippedWeapon->Equip(
					Character->GetMesh(),
					TEXT("WeaponSocket"), // 소켓 이름 확인 필요!
					Character,
					Character
				);
			}

			// 처음엔 숨겨둠 (장착 모션 전까지)
			EquippedWeapon->SetActorHiddenInGame(true);
			bIsPistolEquipped = false;
		}
	}
}

void UPlayerCombatComponent::ToggleEquip(UAnimMontage* Montage, UAnimInstance* AnimInst)
{
	if (!EquippedWeapon) return;
	if (bIsAiming) return;

	bIsPistolEquipped = !bIsPistolEquipped;

	// 1. 무기 보이게/안보이게 설정
	// 해제는 UnEquip 노티파이에서 처리
	if (bIsPistolEquipped)
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
	if (!bIsPistolEquipped || !EquippedWeapon || EquippedWeapon->IsReloading())
	{		
		return false;
	}

	bIsAiming = true;
	
	UE_LOG(LogTemp, Warning, TEXT("CombatComp: bIsAiming set to TRUE"));

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

	if (!EquippedWeapon->HasAmmo())
	{
		EquippedWeapon->PlayDryFireSound();
		return;
	}

	// 1. [Component 역할] 애니메이션 재생 (플레이어의 행동)
	if (AnimInst && FireMontage)
	{
		AnimInst->Montage_Play(FireMontage);
	}

	// 2. [Component 역할] 카메라 쉐이크 (플레이어가 느끼는 반동)
	if (CamShake && GetWorld())
	{
		UGameplayStatics::PlayWorldCameraShake(GetWorld(), CamShake, PlayerCamera->GetComponentLocation(), 0.0f, 500.0f);
	}

	float RecoilPitch = FMath::RandRange(MaxRecoilPitch, MinRecoilPitch);
	float RecoilYaw = FMath::RandRange(MinRecoilYaw, MaxRecoilYaw);

	TargetRecoilRot.Pitch += RecoilPitch;

	TargetRecoilRot.Yaw += RecoilYaw;

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
	if (EquippedWeapon && bIsPistolEquipped)
	{
		// Weapon 클래스에 GetLaserTargetLocation() 함수가 있다고 가정
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