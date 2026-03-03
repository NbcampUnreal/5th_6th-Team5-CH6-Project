// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DefaultAttributeSet_BossMonster.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class WARD_ZERO_API UDefaultAttributeSet_BossMonster : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UDefaultAttributeSet_BossMonster();
	
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing=OnRep_Health,Category="Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UDefaultAttributeSet_BossMonster,Health);
	
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
	
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing=OnRep_MaxHealth,Category="Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UDefaultAttributeSet_BossMonster,MaxHealth);
	
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing=OnRep_BaseSpeed,Category="Attributes")
	FGameplayAttributeData BaseSpeed;
	ATTRIBUTE_ACCESSORS(UDefaultAttributeSet_BossMonster,BaseSpeed);
	
	UFUNCTION()
	virtual void OnRep_BaseSpeed(const FGameplayAttributeData& OldBaseSpeed);
	
	UPROPERTY(BlueprintReadOnly,Category="Attributes")
	FGameplayAttributeData InComingDamage;
	ATTRIBUTE_ACCESSORS(UDefaultAttributeSet_BossMonster,InComingDamage);
};
