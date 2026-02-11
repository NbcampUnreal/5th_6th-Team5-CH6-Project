#include "Character/Components/PlayerCombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

UPlayerCombatComponent::UPlayerCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true; 
}

void UPlayerCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerCombatComponent::SetupCombat(UStaticMeshComponent* InPistolMesh, UCameraComponent* InCamera)
{
	PistolMesh = InPistolMesh;
	PlayerCamera = InCamera;

	// 레이저 사이트 생성
	if (PistolMesh)
	{
		LaserSightComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			LaserSightSystem,
			PistolMesh,
			TEXT("Muzzle"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			false
		);
	}
}

void UPlayerCombatComponent::ToggleEquip(UAnimMontage* EquipMontage, UAnimInstance* AnimInst)
{
	if (!PistolMesh) return;
	if (bIsAiming) return;

	bIsPistolEquipped = !bIsPistolEquipped;
	UE_LOG(LogTemp, Warning, TEXT("%d"), bIsPistolEquipped);
	// 애니메이션 재생
	if (AnimInst && EquipMontage)
	{
		// 총을 꺼낼 때만 몽타주 재생
		if (bIsPistolEquipped)
		{
			AnimInst->Montage_Play(EquipMontage);
		}
	}

	PistolMesh->SetVisibility(bIsPistolEquipped);
}

void UPlayerCombatComponent::StartAiming()
{
	if (bIsPistolEquipped)
	{
		bIsAiming = true;
		if (LaserSightComponent) LaserSightComponent->Activate(true);
	}
}

void UPlayerCombatComponent::StopAiming()
{
	bIsAiming = false;
	if (LaserSightComponent) LaserSightComponent->Deactivate();
}

void UPlayerCombatComponent::Fire(UAnimMontage* FireMontage, UAnimInstance* AnimInst, TSubclassOf<UCameraShakeBase> CamShake)
{
	if (!bIsAiming || !PistolMesh || !PlayerCamera) return;

	// 이펙트
	if (MuzzleFlash)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlash, PistolMesh, TEXT("Muzzle"),
			FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
	}

	// 몽타주
	if (AnimInst && FireMontage)
	{
		AnimInst->Montage_Play(FireMontage);
	}

	// 카메라 쉐이크
	if (CamShake && GetWorld())
	{
		UGameplayStatics::PlayWorldCameraShake(GetWorld(), CamShake, PlayerCamera->GetComponentLocation(), 0.0f, 500.0f);
	}

	// 레이캐스트 (공격)
	FVector Start = PlayerCamera->GetComponentLocation();
	FVector End = Start + (PlayerCamera->GetForwardVector() * 5000.f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner()); 

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (ImpactEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		}

		// 데미지 전달
		UGameplayStatics::ApplyPointDamage(
			Hit.GetActor(),
			10.0f, // 기본 데미지
			(End - Start).GetSafeNormal(),
			Hit,
			GetOwner()->GetInstigatorController(),
			GetOwner(),
			UDamageType::StaticClass()
		);
	}
}

void UPlayerCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsAiming)
	{
		UpdateLaserSight();
		CalculateAimOffset();
	}
}

void UPlayerCombatComponent::UpdateLaserSight()
{
	if (!bIsAiming || !PistolMesh || !LaserSightComponent) return;

	// 총구 위치
	FVector MuzzleLoc = PistolMesh->GetSocketLocation(TEXT("Muzzle"));

	// 카메라 중앙에서 레이캐스트
	FVector CameraLoc = PlayerCamera->GetComponentLocation();
	FVector CameraForward = PlayerCamera->GetForwardVector();
	FVector TraceEnd = CameraLoc + (CameraForward * 10000.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;

	Params.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit, CameraLoc, TraceEnd, ECC_Visibility, Params))
	{
		TraceEnd = Hit.ImpactPoint;
	}

	// IK 타겟 설정
	HandIKTargetLocation = TraceEnd;

	// 레이저는 총구에서 타겟까지
	LaserSightComponent->SetNiagaraVariableVec3(TEXT("BeamEnd"), TraceEnd);
}

void UPlayerCombatComponent::CalculateAimOffset()
{
	if (!bIsAiming) return;

	FVector CameraForward = PlayerCamera->GetForwardVector();

	// [수정] 컴포넌트에는 GetActorRotation이 없습니다. 주인(Owner)에게 물어봐야 합니다.
	FRotator CharacterRotation = GetOwner()->GetActorRotation();
	FVector CharacterForward = CharacterRotation.Vector();

	FRotator DeltaRotation = (CameraForward.Rotation() - CharacterRotation);
	DeltaRotation.Normalize();

	AimYaw = DeltaRotation.Yaw;
	AimPitch = DeltaRotation.Pitch;

	AimYaw = FMath::Clamp(AimYaw, -90.0f, 90.0f);
	AimPitch = FMath::Clamp(AimPitch, -90.0f, 90.0f);
}
