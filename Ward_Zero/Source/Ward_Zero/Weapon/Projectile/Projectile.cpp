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
	UNiagaraSystem* Effect = nullptr;

	if (ProjectileData->ImpactEffectMap.Contains(SurfaceType_Default))
	{
		Effect = ProjectileData->ImpactEffectMap[SurfaceType_Default];
	}

	// 표면별 이펙트 선택
	if (ProjectileData->ImpactEffectMap.Contains(SurfaceType))
		Effect = ProjectileData->ImpactEffectMap[SurfaceType];

	if (Effect) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Effect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());

	UGameplayStatics::ApplyPointDamage(OtherActor, ProjectileData->Damage, GetActorForwardVector(), Hit, GetInstigatorController(), this, ProjectileData->DamageTypeClass);

	Destroy();
}
