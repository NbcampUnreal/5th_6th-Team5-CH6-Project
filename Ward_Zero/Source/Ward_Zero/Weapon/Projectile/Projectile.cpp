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
#include "GameplayTagsManager.h"

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

FGameplayTag AProjectile::GetTagFromSurfaceType(EPhysicalSurface SurfaceType)
{
    //Ex = SurfaceType1이 Metal이라면 "Surface.Metal" 태그를 반환
    FName TagName = TEXT("Surface.Default");

    //Test용 
    //switch (SurfaceType)
    //{
    //case SurfaceType1: TagName = TEXT("Surface.Concrete"); break; // 1층 바닥
    //case SurfaceType2: TagName = TEXT("Surface.Metal"); break;    // 환풍구
    //case SurfaceType3: TagName = TEXT("Surface.Marble"); break;   // 2층 바닥
    //}

    // Tag Manager 에서 이름을 찾아 반환
   /* return UGameplayTagsManager::Get().RequestGameplayTag(TagName);*/
    return FGameplayTag::EmptyTag;
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!ProjectileData) return;

    if (OtherActor && (OtherActor != GetOwner()))
    {
        // Material 기반 태그 추출
        EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);
        FGameplayTag SurfaceTag = GetTagFromSurfaceType(SurfaceType);

        // 기본 이펙트 설정 
        UNiagaraSystem* EffectToSpawn = ProjectileData->DefaultImpactEffect;

        // Material 종류에 따른 이펙트 결정
       /* if (SurfaceTag.IsValid() && ProjectileData->ImpactEffectTagMap.Contains(SurfaceTag))
        {
            EffectToSpawn = ProjectileData->ImpactEffectTagMap[SurfaceTag];
        }*/
        if (OtherActor->ActorHasTag(TEXT("Zombie")))
        {
            EffectToSpawn = ProjectileData->ImpactEffect;
        }

        // 이펙트 생성
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
        USoundBase* SoundToPlay = ProjectileData->DefaultImpactSound;
        if (SurfaceTag.IsValid() && ProjectileData->ImpactSoundTagMap.Contains(SurfaceTag))
        {
            SoundToPlay = ProjectileData->ImpactSoundTagMap[SurfaceTag];
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
