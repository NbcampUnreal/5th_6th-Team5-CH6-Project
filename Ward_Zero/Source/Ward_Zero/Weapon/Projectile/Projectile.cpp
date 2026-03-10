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
	if (!ProjectileData || OtherActor == GetOwner()) return;

	EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);

	// 1. VFX (이펙트) 로직
	UNiagaraSystem* Effect = ProjectileData->ImpactEffectMap.Contains(SurfaceType)
		? ProjectileData->ImpactEffectMap[SurfaceType]
		: ProjectileData->ImpactEffectMap.FindRef(SurfaceType_Default);

	if (Effect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Effect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}

	// 2. SFX (사운드) 로직 추가! ◀ 이 부분이 빠져있었습니다.
	if (ProjectileData->ImpactSoundMap.Contains(SurfaceType) || ProjectileData->ImpactSoundMap.Contains(SurfaceType_Default))
	{
		USoundBase* Sound = ProjectileData->ImpactSoundMap.Contains(SurfaceType)
			? ProjectileData->ImpactSoundMap[SurfaceType]
			: ProjectileData->ImpactSoundMap[SurfaceType_Default];

		if (Sound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, Hit.ImpactPoint);
		}
	}

	// 데미지 처리 및 제거
	UGameplayStatics::ApplyPointDamage(OtherActor, ProjectileData->Damage, GetActorForwardVector(), Hit, GetInstigatorController(), this, ProjectileData->DamageTypeClass);

	Destroy();
}
