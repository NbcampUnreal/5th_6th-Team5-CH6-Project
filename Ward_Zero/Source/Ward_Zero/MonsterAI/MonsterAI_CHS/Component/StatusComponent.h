// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MonsterAI/MonsterAI_CHS/Data/Type/MonsterStat.h"
#include "MonsterAI/MonsterAI_CHS/Data/Type/GameTypes.h"
#include "StatusComponent.generated.h"


class UMonsterDataAsset;
class ABaseZombie;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterMainStateChanged, EMonsterMainState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterSubStateChanged, EMonsterSubState, NewState);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WARD_ZERO_API UStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UStatusComponent();
	virtual void BeginPlay() override;
	void InitData(const UMonsterDataAsset* BaseData);
	bool IsDataInit() const;
	
	void SetMainState(EMonsterMainState NewState);
	void SetSubState(EMonsterSubState NewState);
	EMonsterMainState GetMainState() const;
	EMonsterSubState GetSubState() const;
	EMonsterMainState GetStartState() const;
	
	float GetBaseSpeed() const;
	void SetBaseSpeed(float speed);
	float GetChaseSpeed() const;
	void SetChaseSpeed(float speed);
	float GetArrivalRadius() const;
	float GetBaseDetectionRange() const;
	float GetLoseSightRange() const;
	float GetViewAngle() const;
	float GetAttackRange() const;
	
	float GetEyeHeight() const;
	float GetMaxLostTargetTime() const;
	float GetIdleSoundVolume() const;
	float GetChaseSoundVolume() const;
	float GetHearingThreshold() const;
	
	float GetResistKnockdown() const;
	float GetAttackDamage() const;
	float GetCurrentHP() const;
	float GetHeadHitStunnedTime() const;
	float GetBodyHitStunnedTime() const;
	
	float GetWeakSpotDamageMultiplier() const;
	bool GetIsRecoveringCC() const;
	bool SetIsRecoveringCC(bool b);
	bool GetIsKnockdownSuperArmor() const;
	
	UPROPERTY(BlueprintAssignable)
	FOnMonsterMainStateChanged OnMainStateChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnMonsterSubStateChanged OnSubStateChanged;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	EMonsterMainState StartState = EMonsterMainState::Idle;
	
	float ApplyDamage(float Amount, bool bIsCritical);
	bool GetIsDead() const;
	void SetIsDead(bool b);
	
private:
	UPROPERTY()
	ABaseZombie* OwnerMonster;
	UPROPERTY()
	const UMonsterDataAsset* MonsterData;
	
	
	
	UPROPERTY(VisibleAnywhere)
	EMonsterMainState MainState = EMonsterMainState::Idle;
	UPROPERTY(VisibleAnywhere)
	EMonsterSubState SubState = EMonsterSubState::None;
	bool bIsDataInit = false;
	
	FHealth Health;
	FSpeed Speed;
	
	UPROPERTY()
	bool bIsStunned = false;
	UPROPERTY()
	bool bIsDead = false;
	UPROPERTY()
	bool bIsRecoveringCC = false;
};
