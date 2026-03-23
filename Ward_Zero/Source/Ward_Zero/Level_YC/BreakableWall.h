#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h" 
#include "BreakableWall.generated.h"

UCLASS()
class WARD_ZERO_API ABreakableWall : public AActor
{
    GENERATED_BODY()

public:
    ABreakableWall();
	// 트리거와 충돌했을 때 벽이 파괴되는 함수 (AI 트리거 호출)
    UFUNCTION(BlueprintCallable, Category = "Chaos|Destruction")
    void ExecuteWallDestruction(FVector ImpactLocation);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UGeometryCollectionComponent* GCComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UFieldSystemComponent* FieldSystemComponent;

    UPROPERTY(EditAnywhere, Category = "Chaos|Effects")
    UNiagaraSystem* DustEffect;

    UPROPERTY(EditAnywhere, Category = "Chaos|Effects")
    USoundBase* BreakSound;

    UPROPERTY(EditAnywhere, Category = "Chaos|Settings")
    float DamageRadius = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Chaos|Settings")
    float StrainMagnitude = 500000.0f;

    UPROPERTY(EditAnywhere, Category = "Chaos|Impulse", meta = (MakeEditWidget = true))
    FVector ImpactDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, Category = "Chaos|Impulse")
    float PushMagnitude = 5000000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos|Impulse")
    float PushPowerMultiplier = 1.0f;

public:
    static constexpr float PushForce = 1000.0f;

private:
    bool bIsAlreadyBroken = false;
};