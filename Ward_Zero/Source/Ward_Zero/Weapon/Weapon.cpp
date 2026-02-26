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

        // 만약 크로스헤어와 쏘는 곳이 맞는지 확인할때 필요하면 쓰세요 :)
        /*
        DrawDebugSphere(
            GetWorld(),
            FireHit.ImpactPoint,
            10.0f,               
            12,                  
            FColor::Red,        
            false,               
            5.0f
        );*/

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

    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    }

    if (Mesh && MuzzleFlash)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            MuzzleFlash,
            Mesh,
            MuzzleSocketName, // "MuzzleFlash" 소켓 이름
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget, // 소켓 위치와 회전에 딱 맞춤
            true // Auto Destroy (끝나면 자동 삭제)
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
    if (bIsReloading || CurrentAmmo >= MaxCapacity) return;

    bIsReloading = true;
    PlayReloadSound();
    UE_LOG(LogTemp, Warning, TEXT("Reload Started..."));
}

void AWeapon::FinishReload()
{
    bIsReloading = false;
    CurrentAmmo = MaxCapacity;

    ShowMagazine();

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

void AWeapon::PlayReloadSound()
{
    if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ReloadSound, GetActorLocation());
    }
	UE_LOG(LogTemp, Warning, TEXT("장전 소리"));
}

//기존 탄창이 사라질 때 노티파이 이벤트 
void AWeapon::HideMagazine()
{
    //총에 붙어있는 탄창 숨기기 
    if (GunMagMesh)
    {
        GunMagMesh->SetVisibility(false);
    }

    if (MagazineClass)
    {
        AMagazineBase* DropMag = GetWorld()->SpawnActor<AMagazineBase>
            (
                MagazineClass,
                GunMagMesh->GetComponentLocation(),
                GunMagMesh->GetComponentRotation()
            );

        if (DropMag)
        {
            DropMag->Drop();
        }
    }

    //캐릭터 손에 있는 탄창을 권총 탄창에 스폰해서 붙이기.
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (OwnerChar && MagazineClass)
    {
        CurrHandMag = GetWorld()->SpawnActor<AMagazineBase>(MagazineClass);
        if (CurrHandMag)
        {
            CurrHandMag->AttachToComponent(
                OwnerChar->GetMesh(),
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                TEXT("MagSocket")
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
}