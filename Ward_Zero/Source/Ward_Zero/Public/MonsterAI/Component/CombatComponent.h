// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MonsterAI/Data/MonsterDataAsset.h"
#include "CombatComponent.generated.h"


class UMonsterDataAsset;
class UStatusComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WARD_ZERO_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatComponent();
	void OnTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	void Attack();
	
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void ApplyKnockdown(EHitDirection HitDir);
	void ApplyStun(EHitDirection HitDir, bool bIsCriticalHit);
	bool CheckCriticalHit(const FDamageEvent& DamageEvent);
	void OnDeath();
	
	EHitDirection GetHitDirection(const FVector& ShotDirection);
	
	UPROPERTY()
	TObjectPtr<UStatusComponent> StatusComp;
	
	UPROPERTY()
	const UMonsterDataAsset* MonsterData;


public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};


