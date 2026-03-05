// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "MonsterAI/MonsterAI_CHS/Data/Type/MonsterStat.h"
#include "MonsterAI/MonsterAI_CHS/Data/Type/GameTypes.h"
#include "MonsterDataAsset.generated.h"

/**
 * 
 */
class UAnimMontage;
class USkeletalMesh;
class UAnimInstance;
class USoundBase;

USTRUCT(BlueprintType)
struct FLegHitReactionMontage
{
	GENERATED_BODY()
	public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> LeftLegHitReaction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> RightLegHitReaction;
	
};

USTRUCT(BlueprintType)
struct FDirectionalMontage
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Front; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Back;   

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Left;   

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> Right;  
};

USTRUCT(BlueprintType)
struct FGetUpMontage
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> FromFaceDown; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> FromFaceUp;   
};

USTRUCT(BlueprintType)
struct FInteractionInfo
{
	GENERATED_BODY()

	FInteractionInfo() : InteractionMontage(nullptr), InteractableObject(EInteractableObject::Door), InteractingDuration(2.0f) {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> InteractionMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EInteractableObject InteractableObject;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InteractingDuration;
	
};
UCLASS()
class WARD_ZERO_API UMonsterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
	
	public:
	UPROPERTY(EditAnywhere, Category = "Visual")
	TObjectPtr<USkeletalMesh> MonsterMesh;
	
	UPROPERTY(EditAnywhere, Category = "Visual")
	FVector MeshScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, Category = "Animation")
	TSubclassOf<UAnimInstance> AnimBP;
	
	UPROPERTY(EditAnywhere, Category = "Animation|Idle")
	TObjectPtr<UAnimMontage> IdleMontage;
	UPROPERTY(EditAnywhere, Category = "Animation|Combat")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;
	UPROPERTY(EditAnywhere, Category = "Animation|Chase")
	TObjectPtr<UAnimMontage> BangTheDoorMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	FDirectionalMontage CriticalHitReactMontages; 

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	FDirectionalMontage NormalHitReactMontages; 
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	FLegHitReactionMontage LegHitReactionMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	FDirectionalMontage KnockdownMontages; 

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	FGetUpMontage GetUpMontages;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Effects")
	TObjectPtr<UParticleSystem> BloodEffectSystem;
	
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TMap<FGameplayTag,FInteractionInfo> InteractionInfoMap;
	
	UPROPERTY(EditAnywhere, Category = "State")
	TMap<EMonsterMainState, FMonsterStateSettings> StateConfigMap;
	 
	
	UPROPERTY(EditAnywhere, Category = "Status")
	float MaxHP = 100.f;
	
	UPROPERTY(EditAnywhere, Category = "Chase")
	float ArrivalRadius = 100.f;
	

	UPROPERTY(EditAnywhere, Category = "Sight")
	float BaseDetectionRange = 1200.f;
	
	UPROPERTY(EditAnywhere, Category = "Sight")
	float LoseSightRange = 1500.f;
	
	UPROPERTY(EditAnywhere, Category = "Sight")
	float ViewAngle = 90.f;
	
	UPROPERTY(EditAnywhere, Category = "Hearing", meta=(ClampMin="0.0", ClampMax="1.0"))
	float HearingThreshold = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "Combat|Attack")
	float AttackRange = 150.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Attack")
	float AttackDamage = 30.f;
	
	
	/*UPROPERTY(EditAnywhere, Category = "Chase")
	float ChaseRange = 2000.f;*/
	
	UPROPERTY(EditAnywhere, Category = "Combat|Stun")
	float HeadHitStunnedTime = 1.f;
	
	UPROPERTY(EditAnywhere, Category = "Combat|Stun")
	float BodyHitStunnedTime = 0.f;
	
	/*UPROPERTY(EditAnywhere, Category = "Combat|WeakPoint")
	FName WeakBoneName = FName("head");
	
	UPROPERTY(EditAnywhere, Category = "Combat|WeakPoint")
	FName LeftLegBoneName = FName("left");
	
	UPROPERTY(EditAnywhere, Category = "Combat|WeakPoint")
	FName RightLegBoneName = FName("right");*/
	
	UPROPERTY(EditAnywhere, Category = "Combat|WeakPoint")
	float WeakSpotDamageMultiplier = 2.0f;
	
	UPROPERTY(EditAnywhere, Category = "Combat|Knockdown",meta=(ClampMin="0.0", ClampMax="1.0"))
	float ResistKnockdown = 0.5f;
			
	UPROPERTY(EditAnywhere, Category = "Combat|Knockdown")
	bool bIsKnockdownSuperArmor = false;
	
	UPROPERTY(EditAnywhere, Category = "Sight")
	float EyeHeight = 70.f;
	
	UPROPERTY(EditAnywhere, Category = "Chase")
	float MaxLostTargetTime = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> BulletHitSound;
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> WalkSound;
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> RunSound;
	
	
	
};
