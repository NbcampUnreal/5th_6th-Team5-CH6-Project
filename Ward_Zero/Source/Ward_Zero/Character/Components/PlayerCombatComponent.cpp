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

	UE_LOG(LogTemp, Warning, TEXT("bIsPistolEquipped : %d"), bIsPistolEquipped);
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

	// 레이캐스트 (카메라 중앙 기준)
	FVector CameraStart = PlayerCamera->GetComponentLocation();
	FVector CameraEnd = CameraStart + (PlayerCamera->GetForwardVector() * 5000.f);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CameraStart, CameraEnd, ECC_Visibility, Params);

	// 실제 타격 지점 결정
	FVector ImpactPoint = bHit ? Hit.ImpactPoint : CameraEnd;

	// 총구 위치
	FVector MuzzleLocation = PistolMesh->GetSocketLocation(TEXT("Muzzle"));

	// 총구에서 타격지점으로 향하는 방향 계산
	FVector MuzzleToImpact = (ImpactPoint - MuzzleLocation).GetSafeNormal();

	// 이펙트 - 총구에서 실제 타격 방향으로
	if (MuzzleFlash)
	{
		// 총구 이펙트는 타격 지점을 향하도록 회전
		FRotator MuzzleRotation = MuzzleToImpact.Rotation();
		UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlash, PistolMesh, TEXT("Muzzle"),
			FVector::ZeroVector, MuzzleRotation, EAttachLocation::KeepRelativeOffset, true);
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

	if (bHit)
	{
	/*	DrawDebugLine(GetWorld(), CameraStart, Hit.ImpactPoint, FColor::Blue, false, 2.0f, 0, 0.5f);*/
		DrawDebugLine(GetWorld(), MuzzleLocation, Hit.ImpactPoint, FColor::Yellow, false, 2.0f, 0, 0.5f);
		FColor ImpactColor = bHit ? FColor::Red : FColor::Yellow;
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 5.0f, 8, ImpactColor, false, 2.0f, 0, 1.0f);

		UE_LOG(LogTemp, Warning, TEXT("Hit: %s | Distance: %.1f"),
			*Hit.GetActor()->GetName(), Hit.Distance);
	}
	else
	{
		DrawDebugLine(GetWorld(), CameraStart, CameraEnd, FColor::Red, false, 2.0f, 0, 0.5f);
	}

	// 충돌 이펙트 및 데미지
	if (bHit)
	{
		if (ImpactEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactEffect,
				Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
		}

		// 데미지는 카메라 방향 기준
		UGameplayStatics::ApplyPointDamage(
			Hit.GetActor(),
			10.0f,
			(CameraEnd - CameraStart).GetSafeNormal(), // 카메라 방향
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
