// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	
	bool bIsResistingCC = false;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void ApplyKnockdown(bool bIsKnockdown);
	bool CheckHeadShot(const FDamageEvent& DamageEvent);
	
	UPROPERTY()
	TObjectPtr<UStatusComponent> StatusComp;
	
	UPROPERTY()
	TObjectPtr<UMonsterDataAsset> MonsterData;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};


