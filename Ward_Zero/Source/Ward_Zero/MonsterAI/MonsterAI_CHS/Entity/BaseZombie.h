// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MonsterAI/MonsterAI_CHS/Data/Type/MonsterStat.h"
#include "BaseZombie.generated.h"

enum class EHitDirection : uint8;
class UCombatComponent;
class UStatusComponent;
class UMonsterDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActivateDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRagdollDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecoverDelegate);


UCLASS()
class WARD_ZERO_API ABaseZombie : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseZombie();

	virtual void OnDeath(FVector HitDir, float HitForce);
	virtual void Tick(float DeltaSeconds) override;
	bool bIsActivated = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	const UMonsterDataAsset* MonsterData;
	
	UAudioComponent* GetAudioLoopComponent() const { return AudioLoopComponent; }
	UCombatComponent* GetCombatComponent() const { return CombatComponent;}
	UStatusComponent* GetStatusComponent() const { return StatusComponent; }
	
	UFUNCTION(CallInEditor, Category = "Data")
	void RefreshMonster();
	
	UFUNCTION(BlueprintCallable, Category = "Status", meta=(DisplayName="Get Monster Base Speed"))
	float GetBaseSpeed();
	
	UFUNCTION(BlueprintCallable, Category = "Status", meta=(DisplayName="Set Monster Base Speed"))
	void SetBaseSpeed(float NewSpeed);
	
	UFUNCTION(BlueprintCallable, Category = "Status", meta=(DisplayName="Get Monster Chase Speed"))
	float GetChaseSpeed();
	
	UFUNCTION(BlueprintCallable, Category = "Status", meta=(DisplayName="Set Monster Chase Speed"))
	void SetChaseSpeed(float NewSpeed);
	
	UFUNCTION(BlueprintCallable, Category = "Animation", meta=(DisplayName ="Play Anim Montage"))
	void PlayAnimM(UAnimMontage* MontageToPlay);
	
	UFUNCTION(BlueprintCallable, Category = "Animation", meta=(DisplayName="Bang the Door"))
	void BangDoor(AActor* TargetDoor);
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnActivateDelegate OnActivate;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRagdollDelegate OnRagdoll;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRagdollDelegate OnRecover;
	
	UFUNCTION(BlueprintCallable, Category = "Activate", meta = (DisplayName="Activate Zombie"))
	void Activate();
	void StartRagdollKnockdown(FVector HitDir,FName BoneName,float HitForce);
	void CheckRagdollVelocity();
	void RecoverFromRagdoll();
	
	
	
	const UMonsterDataAsset* GetMonsterData() const;
#if WITH_EDITOR
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	UFUNCTION()
	void HandleStateChange(EMonsterMainState NewState);
	virtual void BeginPlay() override;
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UStatusComponent* StatusComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UAudioComponent* AudioLoopComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UCombatComponent* CombatComponent;
	
	FTimerHandle RagdollTimerHandle;
	
private:
	
	bool bIsExecutionActive = false;
};
