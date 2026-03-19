// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MonsterAI/MonsterAI_CHS/Data/MonsterDataAsset.h"
#include "MonsterAI/MonsterAI_CHS/Weapon/WZDamageType.h"
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
	void ProcessDamageLogic(float Damage, EHitPart HitPart, const FVector& AttackDir, const UWZDamageType* DamageType, AActor* DamageCauser);
	void HandleAllDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	void Attack();
	bool GetIsAttacking();
	void SetIsAttacking(bool isAttacking);
	
	void SpawnHitEffect(FDamageEvent const& DamageEvent);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void ApplyKnockdown(EHitDirection HitDir);
	void ApplyStun(EHitDirection HitDir, EHitPart HitPart);
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
	
	private:
	bool bIsAttacking;
};


