#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "NiagaraSystem.h"
#include "BreakableWall.generated.h"
UCLASS()
class WARD_ZERO_API ABreakableWall : public AActor
{
	GENERATED_BODY()
	
public:
    ABreakableWall();

    // AI 트리거에서 호출할 함수
    UFUNCTION(BlueprintCallable, Category = "Chaos|Destruction")
    void ExecuteWallDestruction(FVector ImpactLocation);

protected:
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UGeometryCollectionComponent* GCComponent;

    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UFieldSystemComponent* FieldSystemComponent;

    
    UPROPERTY(EditAnywhere, Category = "Chaos|Effects")
    UNiagaraSystem* DustEffect;

    
    UPROPERTY(EditAnywhere, Category = "Chaos|Settings")
    float DamageRadius = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Chaos|Settings")
    float StrainMagnitude = 500000.0f; 

   
    bool bIsAlreadyBroken = false;
};