#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ProjectileData.generated.h"

class UDamageType;
class UNiagaraSystem;

UCLASS(BlueprintType)
class WARD_ZERO_API UProjectileData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId("ProjectileData", GetFName()); }

    UPROPERTY(EditAnywhere, Category = "Physics")
    float InitialSpeed = 3000.f;

    UPROPERTY(EditAnywhere, Category = "Physics")
    float MaxSpeed = 3000.f;

    UPROPERTY(EditAnywhere, Category = "Physics")
    float GravityScale = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Physics")
    float LifeSpan = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float Damage = 20.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    TSubclassOf<class UDamageType> DamageTypeClass;

    UPROPERTY(EditAnywhere, Category = "Visual")
    TObjectPtr<class UNiagaraSystem> ImpactEffect;

    UPROPERTY(EditAnywhere, Category = "Visual")
    TObjectPtr<class UNiagaraSystem> TracerEffect;

    // Physics Material 이펙트 매핑
    UPROPERTY(EditAnywhere, Category = "Visual|Impact")
    TMap<TEnumAsByte<EPhysicalSurface>, TObjectPtr<UNiagaraSystem>> ImpactEffectMap;

    UPROPERTY(EditAnywhere, Category = "Visual|Impact")
    TObjectPtr<UNiagaraSystem> DefaultImpactEffect;

    UPROPERTY(EditAnywhere, Category = "Visual|Impact")
    TObjectPtr<UNiagaraSystem> ConcreteImpactEffect; 

    // Physics Material 사운드 매핑 
    UPROPERTY(EditAnywhere, Category = "Sound|Impact")
    TMap<TEnumAsByte<EPhysicalSurface>, TObjectPtr<USoundBase>> ImpactSoundMap;

    UPROPERTY(EditAnywhere, Category = "Visual|Impact")
	TObjectPtr<UNiagaraSystem> MetalImpactEffect;

    UPROPERTY(EditAnywhere, Category = "Visual|Impact")
    TObjectPtr<UNiagaraSystem> MarbelImpactEffect; 


    UPROPERTY(EditAnywhere, Category = "Visual|Sound")
    TObjectPtr<USoundBase> DefaultImpactSoundEffect;

    UPROPERTY(EditAnywhere, Category = "Visual|Sound")
    TObjectPtr<USoundBase> ConcreteImpactSoundEffect;

    UPROPERTY(EditAnywhere, Category = "Visual|Sound")
    TObjectPtr<USoundBase> MetalImpactSoundEffect;

    UPROPERTY(EditAnywhere, Category = "Visual|Sound")
    TObjectPtr<USoundBase> MarbelImpactSoundEffect;
};
