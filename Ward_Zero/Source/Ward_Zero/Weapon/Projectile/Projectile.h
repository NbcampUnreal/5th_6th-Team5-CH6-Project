#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Projectile.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UProjectileData;

UCLASS()
class WARD_ZERO_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	AProjectile();

protected:
	// 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile|Effects")
	TObjectPtr<UNiagaraComponent> TracerComponent;

	// 데이터 에셋 
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<UProjectileData> ProjectileData;

public:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void InitializeProjectile(UProjectileData* Data);
};