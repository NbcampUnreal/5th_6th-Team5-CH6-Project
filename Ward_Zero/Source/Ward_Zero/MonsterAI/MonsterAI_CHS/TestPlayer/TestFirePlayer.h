// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TestFirePlayer.generated.h"

struct FInputActionValue;

UCLASS()
class WARD_ZERO_API ATestFirePlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATestFirePlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UPROPERTY(visibleAnywhere)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, Category= "Input")
	class UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, Category= "Input")
	class UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, Category= "Input")
	class UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category= "Input")
	class UInputAction* ShootAction;
	
	UPROPERTY(EditAnywhere, Category = "Test")
	TSubclassOf<UDamageType> TestDamageType;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Shoot();
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
