#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponData.generated.h"

class UNiagaraSystem;
class USoundBase;
class UProjectileData;

UCLASS(BlueprintType)
class WARD_ZERO_API UWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId("WeaponData", GetFName()); }

    UPROPERTY(EditAnywhere, Category = "Stats")
    float Damage = 20.f;

    UPROPERTY(EditAnywhere, Category = "Stats")
    int32 MaxCapacity = 12;

    UPROPERTY(EditAnywhere, Category = "Stats")
    float FireRate = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Projectile")
    TObjectPtr<class UProjectileData> ProjectileData;

    UPROPERTY(EditAnywhere, Category = "Visual")
    TObjectPtr<class UNiagaraSystem> MuzzleFlash;

    UPROPERTY(EditAnywhere, Category = "Visual")
    TSubclassOf<class AMagazineBase> MagazineClass;

    UPROPERTY(EditAnywhere, Category = "Sound")
    TObjectPtr<USoundBase> FireSound;

    UPROPERTY(EditAnywhere, Category = "Sound")
    TObjectPtr<USoundBase> ReloadSound;

    UPROPERTY(EditAnywhere, Category = "Sound")
    TObjectPtr<USoundBase> DryFireSound;
};
