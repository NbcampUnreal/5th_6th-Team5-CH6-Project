#include "Weapon/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/DamageEvents.h"
#include "MonsterAI/MonsterAI_CHS/Weapon/WZDamageType.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 레이저 포인터 위치 계산 로직
    if (WeaponMesh)
    {
        FVector Start = WeaponMesh->GetSocketLocation(TEXT("Muzzle"));
        // 총구 방향으로 멀리 쏨
        FVector End = Start + (WeaponMesh->GetForwardVector() * 10000.0f);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(GetOwner()); // 총 주인도 무시

        // 레이저 트레이스
        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        {
            LaserHitLocation = Hit.ImpactPoint; // 부딪힌 곳 저장
        }
        else
        {
            LaserHitLocation = End; // 안 부딪히면 허공 끝 저장
        }

        // (선택) 만약 나이아가라 레이저 이펙트가 있다면 여기서 업데이트
        if (LaserSightComponent)
        {
            LaserSightComponent->SetNiagaraVariableVec3(TEXT("BeamEnd"), LaserHitLocation);
        }
    }
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();	

    CurrentAmmo = MaxCapacity;
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
	SetOwner(NewOwner);
	WeaponInstigator = NewInstigator;
	WeaponOwnerController = NewInstigator ? NewInstigator->GetController() : nullptr;

	AttachToComponent(InParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, InSocketName);
}

void AWeapon::Fire(const FVector& HitTarget)
{
    if (!WeaponMesh) return;

    if (CurrentAmmo <= 0)
    {
        PlayDryFireSound();
        return;
    }

    if (bIsReloading) return;

    FVector MuzzleSocketLocation = WeaponMesh->GetSocketLocation(TEXT("Muzzle"));
    FVector OutBeamEnd = HitTarget;

    FHitResult FireHit;
    FVector Start = MuzzleSocketLocation;
    FVector End = HitTarget + (HitTarget - Start).GetSafeNormal() * 50.0f; // 조금 더 길게

    FCollisionQueryParams Params;
    Params.bTraceComplex = false;
    Params.bReturnPhysicalMaterial = true;

    bool bBeamHit = GetWorld()->LineTraceSingleByChannel(
        FireHit, Start, End, ECC_Visibility, Params
    );

    if (bBeamHit)
    {
        OutBeamEnd = FireHit.ImpactPoint;

        if (ImpactEffect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(), ImpactEffect, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation()
            );
        }

        if (FireHit.GetActor())
        {
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *FireHit.GetActor()->GetName());
            UGameplayStatics::ApplyPointDamage(
                FireHit.GetActor(),
                Damage,
                (OutBeamEnd - Start).GetSafeNormal(),
                FireHit,
                WeaponOwnerController,
                this,             // DamageCauser는 무기 자신
                DamageTypeClass   // 여기에 WZDamageType_Gun이 들어감
            );
        }
    }

    if (!bBeamHit)
    {
        OutBeamEnd = MuzzleSocketLocation + (HitTarget - MuzzleSocketLocation).GetSafeNormal() * FireRange;
    }

    UMeshComponent* Mesh = FindComponentByClass<UMeshComponent>();

    if (Mesh && TracerEffect)
    {
        FTransform SocketTransform = Mesh->GetSocketTransform(MuzzleSocketName);
        FVector MuzzleLocation = SocketTransform.GetLocation();

        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
            TracerEffect,
            Mesh,
            MuzzleSocketName,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );

        if (NiagaraComp)
        { 
            NiagaraComp->SetVectorParameter(FName("TracerEnd"), HitTarget);
        }
    }

    if (MuzzleFlash)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            MuzzleFlash, WeaponMesh, TEXT("Muzzle"),
            FVector::ZeroVector, FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset, true
        );
    }

    SpendRound();
}

void AWeapon::SpendRound()
{
    CurrentAmmo = FMath::Clamp(CurrentAmmo - 1, 0, MaxCapacity);

    UE_LOG(LogTemp, Warning, TEXT("Ammo: %d / %d"), CurrentAmmo, MaxCapacity);
}

bool AWeapon::IsEmpty() const
{
    return CurrentAmmo <= 0;
}

void AWeapon::StartReload()
{
    if (bIsReloading || CurrentAmmo >= MaxCapacity) return;

    bIsReloading = true;
    UE_LOG(LogTemp, Warning, TEXT("Reload Started..."));
}

void AWeapon::FinishReload()
{
    bIsReloading = false;
    CurrentAmmo = MaxCapacity;
    UE_LOG(LogTemp, Warning, TEXT("Reload Finished! Ammo Full."));
}

void AWeapon::PlayDryFireSound()
{
    if (DryFireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DryFireSound, GetActorLocation());
    }
    UE_LOG(LogTemp, Warning, TEXT("(총알 없음)"));
}