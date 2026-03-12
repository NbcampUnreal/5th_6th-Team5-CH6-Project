#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

// 전방 선언 (Forward Declarations)
class UWZDamageType;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;
class AMagazineBase;
class UCurveVector;
class AProjectile;
class USpotLightComponent;

UCLASS()
class WARD_ZERO_API AWeapon : public AActor
{
    GENERATED_BODY()

public:
    AWeapon();

protected:
#pragma region 기본 액터 함수 (Actor Overrides)
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
#pragma endregion

#pragma region 데이터 에셋 
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
    TObjectPtr<class UWeaponData> WeaponData;
#pragma endregion 

public:
#pragma region 전투 및 무기 조작 (Combat & Weapon Operations)
    // 무기 발사 (카메라 정보를 밖에서 받아옵니다)
    void Fire(const FVector& HitTarget);

    // 무기 장착/해제
    void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator);

    // 탄약 소비 (Fire에서 호출)
    void SpendRound();

    // 재장전 시작 (상태 변경)
    void StartReload();

    UFUNCTION(BlueprintCallable) void FinishReload();
    UFUNCTION(BlueprintCallable) void HideMagazine();
    UFUNCTION(BlueprintCallable) void ShowMagazine();

    void PlayDryFireSound();
    void PlayReloadSound();

    void FireEffectsOnly();
#pragma endregion

#pragma region 상태 확인 (Getters & State Checks)
    // 탄약이 남아있는지 확인
    bool HasAmmo() const { return CurrentAmmo > 0; }
    bool IsFullAmmo() const { return CurrentAmmo >= MaxCapacity; }
    // 현재 재장전 중인가?
    bool IsReloading() const { return bIsReloading; }
    bool IsAutomatic() const { return bIsAutomatic; }
    float GetFireRate() const { return FireRate; }

    UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
    int32 GetCurrentAmmo() const { return CurrentAmmo; }

    UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
    int32 GetMaxCapacity() const { return MaxCapacity; }

    // IK용 타겟 위치 반환 (레이저 끝점 등)
    FVector GetLaserTargetLocation() const { return LaserHitLocation; }

    UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
    int32 GetReserveAmmo() const { return ReserveAmmo; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
    void AddAmmo(int32 Amount);
#pragma endregion

#pragma region 컴포넌트 (Components)
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon|Components")
    TObjectPtr<USkeletalMeshComponent> WeaponMesh; //SMG 매시 

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon|Components")
    TObjectPtr<UStaticMeshComponent> SMGLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon|Components")
    TObjectPtr<USpotLightComponent> SMGSpotLight;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Reload")
    TSubclassOf<AMagazineBase> MagazineClass; //실제 탄창 

    UPROPERTY()
    AActor* CurrHandMag; //현재 손에 들고있는 탄창 

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> GunMagMesh;//총에 붙어있는 탄창

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Reload")
    FName HandSocketName = TEXT("MagSocket"); //손 탄창 부착한 소켓 

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sockets") //총 탄창 소켓
        FName MagSocketName = TEXT("MagSocket");

    UPROPERTY(EditAnywhere, Category = "Weapon | Effects")
    class UNiagaraSystem* ShellEjectEffect;

    UPROPERTY(EditAnywhere, Category = "Weapon | Sockets")
    FName ShellEjectSocketName = TEXT("ShellEject");
#pragma endregion

protected:
#pragma region 무기 능력치 설정 (Weapon Config & Stats)
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Stats")
    bool bIsAutomatic = false;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Stats")
    float FireRate = 0.5f; // n초마다 1발 발사

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    int32 MaxCapacity = 12; // 탄창 용량

    // 몬스터에게 전달할 데미지 타입
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Stats")
    TSubclassOf<UWZDamageType> DamageTypeClass;

public:
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
    UCurveVector* RecoilCurve;
#pragma endregion

protected:
#pragma region 무기 상태 (Weapon State)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|State")
    int32 CurrentAmmo;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|State")
    bool bIsReloading = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|State")
    int32 ReserveAmmo = 30;

    UPROPERTY()
    float Damage;
#pragma endregion

public:
    UFUNCTION(BlueprintCallable)
    void SetIsReloading(bool reload);

#pragma region 시각/청각 이펙트 (VFX & SFX)
    UPROPERTY(EditAnywhere, Category = "Weapon|Effects")
    FName MuzzleSocketName = TEXT("MuzzleFlash");

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects|VFX")
    UNiagaraSystem* MuzzleFlash;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects|VFX")
    UNiagaraSystem* ImpactEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects|VFX")
    UNiagaraSystem* LaserSightSystem;

    UPROPERTY(EditAnywhere, Category = "Weapon|Effects|VFX")
    UNiagaraSystem* TracerEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects|SFX")
    USoundBase* DryFireSound;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects|SFX")
    USoundBase* FireSound;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects|SFX")
    USoundBase* ReloadSound;

    // 레이저 사이트 및 무기 틱 활성화/비활성화 함수
    UFUNCTION(BlueprintCallable, Category = "Weapon|Effects")
    void EnableLaserSight(bool bEnable);
#pragma endregion

public:
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Projectile")
    TSubclassOf<AProjectile> ProjectileClass;

private:
#pragma region 내부 처리용 변수 (Internal Data)
    UPROPERTY()
    UNiagaraComponent* LaserSightComponent;

    // 공격의 주체 (데미지 전달용)
    UPROPERTY()
    AController* WeaponOwnerController;

    UPROPERTY()
    APawn* WeaponInstigator;

#pragma endregion

private:
    // 최적화 => 매 프레임 Cast를 피하기 위한 캐싱 변수
    UPROPERTY()
    TObjectPtr<class APrototypeCharacter> CachedOwnerCharacter;

    FVector LaserHitLocation;

    // 반동 데이터 Getter 함수 
public:
    UCurveVector* GetRecoilCurve() const;
    float GetRecoilIntensity() const;
    float GetRecoilRecoverySpeed() const;
    TSubclassOf<UCameraShakeBase> GetFireCameraShake() const;

};