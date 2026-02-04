// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusComponent.generated.h"


class UMonsterDataAsset;
class ABaseZombie;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WARD_ZERO_API UStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UStatusComponent();
	virtual void BeginPlay() override;
	void InitData(const UMonsterDataAsset* BaseData);
	
	
	bool GetIsExecutionActive() const { return bIsExecutionActive; }
	void SetIsExecutionActive(bool b){bIsExecutionActive = b;}
	float GetBaseSpeed() const;
	void SetBaseSpeed(float speed);
	float GetChaseSpeed() const;
	void SetChaseSpeed(float speed);
	float GetArrivalRadius() const;
	float GetBaseDetectionRange() const;
	float GetViewAngle() const;
	float GetAttackRange() const;
	float GetChaseRange() const;
	float GetEyeHeight() const;
	float GetMaxLostTargetTime() const;
	float GetIdleSoundVolume() const;
	float GetChaseSoundVolume() const;
	float GetStunTime() const;
	float GetHearingThreshold() const;
	
	
	
private:
	UPROPERTY()
	ABaseZombie* OwnerMonster;
	UPROPERTY()
	const UMonsterDataAsset* MonsterData;
	
	bool bIsExecutionActive = false;
	
	UPROPERTY()
	float CurrentHP = 0.0f;
	UPROPERTY()
	float BaseSpeed = 0.f;
	UPROPERTY()
	float ChaseSpeed = 0.f;
	UPROPERTY()
	bool bIsSpecialActivate = false;
	UPROPERTY()
	bool bIsStunned = false;
	UPROPERTY()
	bool bIsWaiting = false;
};
