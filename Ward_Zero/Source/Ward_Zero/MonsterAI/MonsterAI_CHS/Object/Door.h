// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZombieInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "Door.generated.h"

class UNavModifierComponent;

UCLASS()
class WARD_ZERO_API ADoor : public AActor, public IZombieInteractableInterface
{
	GENERATED_BODY()

public:	
	ADoor();

protected:
	virtual void BeginPlay() override;

public:
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual FOnEntityDestroyed& GetOnDestroyedDelegate() override { return OnDoorDestroyed; }
	virtual float GetEntityHealth() const override;
	virtual ANavLinkProxy* GetLinkedNavProxy() const override { return LinkedProxy; }
	
	
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void OpenDoor();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CloseDoor();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ToggleDoor();

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEntityDestroyed OnDoorDestroyed;

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNavModifierComponent> NavModifier;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	TObjectPtr<ANavLinkProxy> LinkedProxy;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsOpen = false;
	bool bIsBroken = false;

private:
	
	void UpdateNavigationState();
	
	void HandleDestruction();
};
