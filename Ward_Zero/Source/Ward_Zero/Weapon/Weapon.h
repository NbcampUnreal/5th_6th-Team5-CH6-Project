#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

// 몬스터 팀이 만든 데미지 타입을 쓰기 위해 전방 선언
class UWZDamageType;
class UNiagaraSystem;

UCLASS()
class WARD_ZERO_API AWeapon : public AActor
{
    GENERATED_BODY()

public:
    AWeapon();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // 무기 발사 함수 (카메라 정보를 밖에서 받아옵니다)
    void Fire(const FVector& HitTarget);

    // 무기 장착/해제
    void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator);

    // IK용 타겟 위치 반환 (레이저 끝점 등)
    FVector GetLaserTargetLocation() const
    {
        return LaserHitLocation;
    }

protected:
    // 무기 메쉬 (Skeletal or Static) - 여기선 Static으로 가정
    UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
    UStaticMeshComponent* WeaponMesh;

    // [핵심] 몬스터에게 전달할 데미지 타입 (블루프린트에서 WZDamageType_Gun 선택)
    UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
    TSubclassOf<UWZDamageType> DamageTypeClass;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
    float Damage = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
    float FireRange = 5000.0f;

    // 이펙트들
    UPROPERTY(EditDefaultsOnly, Category = "Weapon Effects")
    UNiagaraSystem* MuzzleFlash;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Effects")
    UNiagaraSystem* ImpactEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Effects")
    UNiagaraSystem* LaserSightSystem;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Effects")
    UNiagaraSystem* TraceEffect;

private:
    UPROPERTY()
    class UNiagaraComponent* LaserSightComponent;

    // 공격의 주체 (데미지 전달용)
    UPROPERTY()
    AController* WeaponOwnerController;

    UPROPERTY()
    APawn* WeaponInstigator;

	FVector LaserHitLocation;
};