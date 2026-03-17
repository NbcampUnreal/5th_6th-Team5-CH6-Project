#include "Weapon/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/DamageEvents.h"
#include "MonsterAI/MonsterAI_CHS/Weapon/WZDamageType.h"
#include "GameFramework/Character.h"
#include "Magazine/MagazineBase.h"
#include "Curves/CurveVector.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "Weapon/Data/WeaponData.h"
#include "Weapon/Data/ProjectileData.h"
#include "Weapon/Projectile/Projectile.h"
#include "Components/SpotLightComponent.h"
#include "FlashLight/Data/FlashLightData.h"
#include "Perception/AISense_Hearing.h"
#include "Character/Noise/NoiseFucLibrary/PlayerNoise.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Status/PlayerStatusComponent.h"
//#include "DrawDebugHelpers.h"

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh);
    WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetCastShadow(false);

    GunMagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMagMesh"));
    GunMagMesh->SetupAttachment(WeaponMesh, TEXT("MagSocket"));
    GunMagMesh->SetCastShadow(false);
    GunMagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SMGLight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SMGLight"));
    SMGLight->SetupAttachment(RootComponent);
    SMGLight->SetRelativeLocation(FVector(-31.2f, 0.95f, 2.8f));
    SMGLight->SetRelativeRotation(FRotator(0.0f, 0.0f, 180.0f));
    SMGLight->SetRelativeScale3D(FVector(0.02f, 0.005f, 0.005f));
    SMGLight->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SMGSpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SMGSpotLight"));
    SMGSpotLight->SetupAttachment(SMGLight);
    SMGSpotLight->SetRelativeLocation(FVector(20.0f, 0.0f, 5.0f));
    SMGSpotLight->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

    // 빛 설정
    SMGSpotLight->Intensity = 0.0f;
    SMGSpotLight->OuterConeAngle = 0.0f;
    SMGSpotLight->SetVisibility(false);
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 레이저 포인터 위치 계산 로직
    if (CachedOwnerCharacter && CachedOwnerCharacter->GetEquippedWeapon() == this && CachedOwnerCharacter->GetIsAiming())
    {
        FVector Start = WeaponMesh->GetSocketLocation(TEXT("Muzzle"));
        FVector End = Start + (WeaponMesh->GetForwardVector() * 10000.0f);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(GetOwner());

        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        {
            LaserHitLocation = Hit.ImpactPoint;
        }
        else
        {
            LaserHitLocation = End;
        }

        if (LaserSightComponent)
        {
            LaserSightComponent->SetVariableVec3(TEXT("BeamEnd"), LaserHitLocation);
            LaserSightComponent->SetVisibility(true);
        }
    }
    else if (LaserSightComponent)
    {
        LaserSightComponent->SetVisibility(false);
    }
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    CachedOwnerCharacter = Cast<APrototypeCharacter>(GetOwner());

    if (SMGSpotLight)
    {
        SMGSpotLight->SetVisibility(false);
        SMGSpotLight->Intensity = 0.0f;
    }

    if (WeaponData)
    {
        MaxCapacity = WeaponData->MaxCapacity;
        CurrentAmmo = MaxCapacity;
        Damage = WeaponData->Damage;
        FireRate = WeaponData->FireRate;

        if (WeaponData->FlashlightSettings && SMGSpotLight)
        {
            UFlashLightData* LightData = WeaponData->FlashlightSettings;
            SMGSpotLight->Intensity = LightData->Intensity;
            SMGSpotLight->OuterConeAngle = LightData->OuterConeAngle;
            SMGSpotLight->AttenuationRadius = LightData->AttenuationRadius;

            SMGSpotLight->SetVisibility(false);
        }
    }

    if (SMGLight)
    {
        UMaterialInstanceDynamic* DynMat = SMGLight->CreateAndSetMaterialInstanceDynamic(0);
    }
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
    if (!WeaponData || !ProjectileClass) return;

    if (CurrentAmmo <= 0 || bIsReloading)
    {
        PlayDryFireSound();
        return;
    }

    FireEffectsOnly();

    // 탄약 소모
    SpendRound();
}

void AWeapon::SpendRound()
{
    CurrentAmmo = FMath::Clamp(CurrentAmmo - 1, 0, MaxCapacity);
    UE_LOG(LogTemp, Warning, TEXT("Ammo: %d / %d"), CurrentAmmo, MaxCapacity);
    if (APrototypeCharacter* OwnerChar = Cast<APrototypeCharacter>(GetOwner()))
    {
        if (UPlayerStatusComponent* Status = OwnerChar->StatusComp)
        {
            int32 Idx = (WeaponData->GetFName().ToString().Contains(TEXT("Pistol"))) ? 1 : 2;
            Status->UpdateAmmoUI(Idx, CurrentAmmo, MaxCapacity, ReserveAmmo);
        }
    }
}

void AWeapon::StartReload()
{
    if (bIsReloading || CurrentAmmo >= MaxCapacity || ReserveAmmo <= 0) return;

    bIsReloading = true;
    PlayReloadSound();
    UE_LOG(LogTemp, Warning, TEXT("Reload Started..."));
}

void AWeapon::FinishReload()
{
    bIsReloading = false;

    int32 AmmoNeeded = MaxCapacity - CurrentAmmo;

    if (AmmoNeeded > 0 && ReserveAmmo > 0)
    {
        int32 AmmoToReload = FMath::Min(AmmoNeeded, ReserveAmmo);

        CurrentAmmo += AmmoToReload;
        ReserveAmmo -= AmmoToReload;
    }

    ShowMagazine();

    if (APrototypeCharacter* OwnerChar = Cast<APrototypeCharacter>(GetOwner()))
    {
        if (UPlayerStatusComponent* Status = OwnerChar->StatusComp)
        {
            int32 Idx = (WeaponData->GetFName().ToString().Contains(TEXT("Pistol"))) ? 1 : 2;
            Status->UpdateAmmoUI(Idx, CurrentAmmo, MaxCapacity, ReserveAmmo);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Reload Finished! Ammo: %d, Reserve: %d"), CurrentAmmo, ReserveAmmo);
}

void AWeapon::PlayDryFireSound()
{
    if (WeaponData && WeaponData->DryFireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, WeaponData->DryFireSound, GetActorLocation());
    }
    UE_LOG(LogTemp, Warning, TEXT("(총알 없음)"));
}

void AWeapon::PlayReloadSound()
{
    if (WeaponData && WeaponData->ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, WeaponData->ReloadSound, GetActorLocation());
    }
    UE_LOG(LogTemp, Warning, TEXT("장전 소리"));
}

void AWeapon::FireEffectsOnly()
{
    if (!WeaponData) return;

    // 사운드 재생
    if (WeaponData->FireSound)
        UGameplayStatics::PlaySoundAtLocation(this, WeaponData->FireSound, GetActorLocation());

    // 총구 화염 (Muzzle Flash)
    if (WeaponData->MuzzleFlash)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            WeaponData->MuzzleFlash, WeaponMesh, TEXT("Muzzle"),
            FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
    }

    // 탄피 배출 (Shell Eject)
    if (WeaponData->ShellEjectEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            WeaponData->ShellEjectEffect, WeaponMesh, TEXT("ShellEject"),
            FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
    }

    // 소음 발생 (AI용)
    if (WeaponInstigator && WeaponData)
    {
        UPlayerNoise::ReportNoise(GetWorld(), WeaponInstigator, GetActorLocation(),
            WeaponData->NoiseLoudness, WeaponData->NoiseRange, WeaponData->NoiseTag);
    }
}

void AWeapon::SetIsReloading(bool reload)
{
    bIsReloading = reload;
    return;
}

void AWeapon::EnableLaserSight(bool bEnable)
{
    // 조준 상태에 따라 무기의 틱(LineTrace 연산) 키고 끄기.
    SetActorTickEnabled(bEnable);

    // 틱을 껐을 때(조준 해제 시), 레이저 파티클이 남아있는 경우 끄기 
    if (!bEnable && LaserSightComponent)
    {
        LaserSightComponent->SetVisibility(false);
    }
}

UCurveVector* AWeapon::GetRecoilCurve() const
{
    return WeaponData ? WeaponData->RecoilCurve : nullptr;
}

float AWeapon::GetRecoilIntensity() const
{
    return WeaponData ? WeaponData->RecoilIntensity : 1.0f;
}

TSubclassOf<UCameraShakeBase> AWeapon::GetFireCameraShake() const
{
    return WeaponData ? WeaponData->FireCameraShake : nullptr;
}

//기존 탄창이 사라질 때 노티파이 이벤트 
void AWeapon::HideMagazine()
{
    if (GunMagMesh) GunMagMesh->SetVisibility(false);

    if (WeaponData && WeaponData->MagazineClass && WeaponMesh)
    {
        FVector SpawnLoc = WeaponMesh->GetSocketLocation(MagSocketName);
        FRotator SpawnRot = WeaponMesh->GetSocketRotation(MagSocketName);

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwner();
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AMagazineBase* DropMag = GetWorld()->SpawnActor<AMagazineBase>(WeaponData->MagazineClass, SpawnLoc, SpawnRot, SpawnParams);

        if (DropMag)
        {
            DropMag->Drop();
        }
    }

    // 캐릭터 왼손에 새 탄창 생성
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (OwnerChar && MagazineClass)
    {
        CurrHandMag = GetWorld()->SpawnActor<AMagazineBase>(MagazineClass);
        if (CurrHandMag)
        {
            FName ReloadSocket = (GetOwner()->FindComponentByClass<UPlayerCombatComponent>()->GetCurrentWeaponIndex() == 1)
                ? TEXT("MagSocket") : TEXT("MagSocket_L");

            CurrHandMag->AttachToComponent(
                OwnerChar->GetMesh(),
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                ReloadSocket
            );
        }
    }
}
//새 탄창이 들어갈 때 노티파이 이벤트 
void AWeapon::ShowMagazine()
{
    //손에 있던 탄창 엑터 제거 
    if (CurrHandMag)
    {
        CurrHandMag->Destroy();
        CurrHandMag = nullptr;
    }
    if (GunMagMesh)
    {
        GunMagMesh->SetVisibility(true);
    }
    bIsReloading = false;
}

// 탄약상자 먹을 때 총알 추가
void AWeapon::AddAmmo(int32 Amount)
{
    ReserveAmmo += Amount;

    // 탄창 주울 때 소리 
    if (WeaponData && WeaponData->PickupMagSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, WeaponData->PickupMagSound, GetActorLocation());
    }

    UE_LOG(LogTemp, Warning, TEXT("Ammo Added! Current Reserve: %d"), ReserveAmmo);

    if (APrototypeCharacter* OwnerChar = Cast<APrototypeCharacter>(GetOwner()))
    {
        if (UPlayerStatusComponent* Status = OwnerChar->StatusComp)
        {
            int32 Idx = (WeaponData->GetFName().ToString().Contains(TEXT("Pistol"))) ? 1 : 2;
            Status->UpdateAmmoUI(Idx, CurrentAmmo, MaxCapacity, ReserveAmmo);
        }
    }
}