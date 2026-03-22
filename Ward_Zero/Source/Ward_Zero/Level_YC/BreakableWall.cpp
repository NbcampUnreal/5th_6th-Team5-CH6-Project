#include "Level_YC/BreakableWall.h"
#include "Field/FieldSystemTypes.h"  
#include "Field/FieldSystemNodes.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"    

ABreakableWall::ABreakableWall()
{
    PrimaryActorTick.bCanEverTick = false;

    GCComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GCComponent"));
    RootComponent = GCComponent;

    FieldSystemComponent = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystem"));
}

void ABreakableWall::ExecuteWallDestruction(FVector ImpactLocation)
{
    if (bIsAlreadyBroken || !GCComponent || !FieldSystemComponent) return;
    bIsAlreadyBroken = true;

    // 1. 물리 필드 생성
    URadialFalloff* RadialFalloff = NewObject<URadialFalloff>(this);

    
    RadialFalloff->SetRadialFalloff(
        StrainMagnitude,
        0.0f,
        1.0f,
        0.0f,
        DamageRadius,
        ImpactLocation,
        (EFieldFalloffType)0 
    );

    // 클러스터 파괴 스트레인 적용
    FieldSystemComponent->ApplyPhysicsField(true, EFieldPhysicsType::Field_ExternalClusterStrain, nullptr, RadialFalloff);

    // 나이아가라 이펙트
    if (DustEffect)
    {
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            DustEffect,
            ImpactLocation,
            FRotator::ZeroRotator,
            FVector(1.0f),
            true, // bAutoDestroy
            true  // bAutoActivate
        );
    }
}