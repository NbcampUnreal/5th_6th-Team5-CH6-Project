// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseZombie.generated.h"

class UStatusComponent;
class UMonsterDataAsset;

UCLASS()
class WARD_ZERO_API ABaseZombie : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseZombie();

virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	const UMonsterDataAsset* MonsterData;
	
	UAudioComponent* GetAudioLoopComponent() const { return AudioLoopComponent; }
	
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
	
#if WITH_EDITOR
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UStatusComponent* StatusComponent;
	
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UAudioComponent* AudioLoopComponent;
	

	
private:
	bool bIsExecutionActive = false;
};
