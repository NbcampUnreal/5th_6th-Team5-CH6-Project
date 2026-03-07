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
#include "Character/Components/PlayerCombatComponent.h"
#include "Weapon/Data/WeaponData.h"
#include "Weapon/Data/ProjectileData.h"
#include "Weapon/Projectile/Projectile.h"
#include "Components/SpotLightComponent.h"
#include "FlashLight/Data/FlashLightData.h"
//#include "DrawDebugHelpers.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    GunMagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMagMesh"));
    GunMagMesh->SetupAttachment(WeaponMesh, TEXT("MagSocket"));

    SMGLight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SMGLight"));
    SMGLight->SetupAttachment(RootComponent);
    SMGLight->SetRelativeLocation(FVector(-31.2f, 0.95f, 2.8f));
    SMGLight->SetRelativeRotation(FRotator(0.0f, 0.0f, 180.0f));
    SMGLight->SetRelativeScale3D(FVector(0.02f, 0.005f, 0.005f));

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
            LaserSightComponent->SetVariableVec3(TEXT("BeamEnd"), LaserHitLocation);
        }
    }
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();	

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

    // 발사 위치 및 회전값 계산
    // 소켓의 위치와 회전값 가져오기 
    FVector MuzzleLocation = WeaponMesh->GetSocketLocation(TEXT("Muzzle"));
    FRotator MuzzleRotation = WeaponMesh->GetSocketRotation(TEXT("Muzzle"));

    // 회전값에서 앞방향(Forward) 벡터를 계산 
    FVector MuzzleForward = FRotationMatrix(MuzzleRotation).GetUnitAxis(EAxis::X);

    // 총구 앞 15cm 지점에서 생성
    FVector SpawnLocation = MuzzleLocation + (MuzzleForward * 15.0f);

    // 생성 지점에서 조준점을 바라보는 회전값 재계산
    FRotator ProjectileRotation = (HitTarget - SpawnLocation).Rotation();

    // 월드에 총알 액터 스폰 
    UWorld* World = GetWorld();
    if (World)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwner();
        SpawnParams.Instigator = Cast<APawn>(GetOwner());
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // 보정된 위치와 조준 방향으로 총알 스폰
        AProjectile* SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, ProjectileRotation, SpawnParams);

        if (SpawnedProjectile && WeaponData->ProjectileData)
        {
            SpawnedProjectile->InitializeProjectile(WeaponData->ProjectileData);
        }
    }

    // 사운드 재생 
    if (WeaponData->FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, WeaponData->FireSound, GetActorLocation());
    }

    if (WeaponData->MuzzleFlash)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            WeaponData->MuzzleFlash,
            WeaponMesh,
            TEXT("Muzzle"), 
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget, true
        );
    }

    if (WeaponData->ShellEjectEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            WeaponData->ShellEjectEffect,
            WeaponMesh,
            TEXT("ShellEject"), // 무기 메시에 미리 만들어둔 소켓 이름
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget, true
        );
    }

    SpendRound();
}

void AWeapon::SpendRound()
{
    CurrentAmmo = FMath::Clamp(CurrentAmmo - 1, 0, MaxCapacity);

    UE_LOG(LogTemp, Warning, TEXT("Ammo: %d / %d"), CurrentAmmo, MaxCapacity);
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

void AWeapon::SetIsReloading(bool reload)
{
    bIsReloading = reload; 
    return;
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
    UE_LOG(LogTemp, Warning, TEXT("Ammo Added! Current Reserve: %d"), ReserveAmmo);
}