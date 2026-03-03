// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "MonsterAI/MonsterAI_CHS/GASMonster/Attribute/DefaultAttributeSet_BossMonster.h"
#include "GASBaseMonster.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChangedDelegate,float,CurrentValue,float,MaxValue);

UCLASS()
class WARD_ZERO_API AGASBaseMonster : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGASBaseMonster();
	
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "GAS")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UDefaultAttributeSet_BossMonster> DefaultAttributeSet_BossMonster;

	void InitializeAttributes();

	virtual void PossessedBy(AController* NewController) override;
	void OnHealthChangedCallback(const FOnAttributeChangeData& Data) const;
	void OnStaminaChangedCallback(const FOnAttributeChangeData& Data) const;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "GAS|Attributes")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;

	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data) const;
	void UpdateMoveSpeed(float NewSpeed) const;
public:
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedDelegate OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedDelegate OnStaminaChanged;
};
