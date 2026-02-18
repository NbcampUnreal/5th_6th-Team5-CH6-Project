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

    // 탄약 소비 (Fire에서 호출)
    void SpendRound();

    // 탄약이 남아있는지 확인
    bool IsEmpty() const;

    // 재장전 시작 (상태 변경)
    void StartReload();

    // [중요] 애니메이션 팀에게 알려줄 함수 (탄약 채우기)
    UFUNCTION(BlueprintCallable)
    void FinishReload();

    // 현재 재장전 중인가?
    bool IsReloading() const { return bIsReloading; }

    // IK용 타겟 위치 반환 (레이저 끝점 등)
    FVector GetLaserTargetLocation() const
    {
        return LaserHitLocation;
    }

    bool HasAmmo() const { return CurrentAmmo > 0; }

    void PlayDryFireSound();

public:
    // 무기 메쉬 (Skeletal or Static) - 여기선 Static으로 가정
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon Properties")
    UStaticMeshComponent* WeaponMesh;
protected:
    // [핵심] 몬스터에게 전달할 데미지 타입 (블루프린트에서 WZDamageType_Gun 선택)
    UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
    TSubclassOf<UWZDamageType> DamageTypeClass;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
    float Damage = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
    float FireRange = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Config")
    int32 MaxCapacity = 12; // 탄창 용량

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Config")
    int32 CurrentAmmo;      // 현재 남은 탄약

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon State")
    bool bIsReloading = false;

    // 이펙트들
    UPROPERTY(EditDefaultsOnly, Category = "Weapon Effects")
    UNiagaraSystem* MuzzleFlash;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Effects")
    UNiagaraSystem* ImpactEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Effects")
    UNiagaraSystem* LaserSightSystem;

    UPROPERTY(EditAnywhere, Category = "Weapon Effects")
    UNiagaraSystem* TracerEffect;

    UPROPERTY(EditAnywhere, Category = "Weapon Effects")
    FName MuzzleSocketName = TEXT("MuzzleFlash");

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Effects")
    USoundBase* DryFireSound;

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