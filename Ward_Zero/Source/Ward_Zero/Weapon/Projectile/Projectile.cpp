#include "Weapon/Projectile/Projectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h" 
#include "GameFramework/ProjectileMovementComponent.h" 
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Data/ProjectileData.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GameplayTagContainer.h"
#include "Chaos/ChaosEngineInterface.h"

AProjectile::AProjectile()
{
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->SetUseCCD(true);
    CollisionComp->bReturnMaterialOnMove = true;
    CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	RootComponent = CollisionComp;
	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->bRotationFollowsVelocity = true;

    //InitializeProjectile에서 덮어씌움. 
    ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = 0.f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

    TracerComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TracerComponent"));
    TracerComponent->SetupAttachment(RootComponent);
}

void AProjectile::InitializeProjectile(UProjectileData* Data)
{
    if (!Data) return;
    ProjectileData = Data;

    // Projectile 데이터 에셋 기반 초기화 
    ProjectileMovement->InitialSpeed = Data->InitialSpeed;
    ProjectileMovement->MaxSpeed = Data->MaxSpeed;
    ProjectileMovement->ProjectileGravityScale = Data->GravityScale;

    // ProjectileMovement의 속도를 실제 물리 엔진에 갱신
    ProjectileMovement->Velocity = GetActorForwardVector() * Data->InitialSpeed;
    SetLifeSpan(Data->LifeSpan);

    if (Data->TracerEffect && TracerComponent)
    {
        TracerComponent->SetAsset(Data->TracerEffect);
    }
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!ProjectileData) return;

    if (OtherActor && (OtherActor != GetOwner()))
    {

        // Material 타입 가져오기 = enum 
        EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);

        UNiagaraSystem* EffectToSpawn = ProjectileData->DefaultImpactEffect;
        USoundBase* SoundToPlay = ProjectileData->DefaultImpactSoundEffect;

        switch (SurfaceType)
        {
        case SurfaceType1:
            if (ProjectileData->ConcreteImpactEffect) EffectToSpawn = ProjectileData->ConcreteImpactEffect;
            if (ProjectileData->ConcreteImpactSoundEffect) SoundToPlay = ProjectileData->ConcreteImpactSoundEffect;
            break;
        case SurfaceType2:
            if (ProjectileData->MetalImpactEffect) EffectToSpawn = ProjectileData->MetalImpactEffect;
            if (ProjectileData->MetalImpactEffect) SoundToPlay = ProjectileData->MetalImpactSoundEffect;
            break;
        case SurfaceType3:
            if (ProjectileData->MarbelImpactEffect) EffectToSpawn = ProjectileData->MarbelImpactEffect;
            if (ProjectileData->MarbelImpactEffect) SoundToPlay = ProjectileData->MarbelImpactSoundEffect;
            break;
        }

        if (OtherActor->ActorHasTag(TEXT("Zombie")))
        {
            EffectToSpawn = ProjectileData->ImpactEffect;
        }
        else if (ProjectileData->ImpactEffectMap.Contains(SurfaceType))
        {
            EffectToSpawn = ProjectileData->ImpactEffectMap[SurfaceType];
        }

        if (EffectToSpawn)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                EffectToSpawn,
                Hit.ImpactPoint,
                Hit.ImpactNormal.Rotation()
            );
        }

        // 사운드 결정 및 재생
        
        if (ProjectileData->ImpactSoundMap.Contains(SurfaceType))
        {
            SoundToPlay = ProjectileData->ImpactSoundMap[SurfaceType];
        }

        if (SoundToPlay)
        {
            UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, Hit.ImpactPoint);
        }

        // 데미지 및 물리 정보 전달
        if (ProjectileData->DamageTypeClass)
        {
            UGameplayStatics::ApplyPointDamage(
                OtherActor,
                ProjectileData->Damage,
                GetActorForwardVector(),
                Hit,
                GetInstigatorController(),
                this,
                ProjectileData->DamageTypeClass
            );
        }
        DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10.0f, 12, FColor::Red, false, 2.0f);
        UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s | Hit Bone: %s"), *OtherActor->GetName(), *Hit.BoneName.ToString());
        Destroy();
    }
}
