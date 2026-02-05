#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PrototypeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;


UCLASS()
class WARD_ZERO_API APrototypeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APrototypeCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    // 카메라 붐
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    // 메인 카메라
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* MainCamera;

    // Enhanced Input Mapping
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    // 이동 액션 (WASD)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    // 시점 조절 액션 (마우스)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* RunAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* CrouchAction;

private:

	bool bIsRunning = false;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 200.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RunSpeed = 250.0f;

    UPROPERTY(EditDefaultsOnly, Category = Movement)
    float CrouchMovementSpeed = 150.0f;

    UPROPERTY(EditDefaultsOnly, Category = Camera)
    float CrouchedArmLength = 130.0f;

    float StandingArmLength = 200.0f;

    UPROPERTY(EditDefaultsOnly, Category = Movement)
    float WalkTurnRate = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = Camera)
    float CrouchedCameraHeight = 40.0f;

    float StandingCameraHeight = 30.0f;

    float CurrentBaseCameraZ = StandingCameraHeight;

    float BobTime = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = Camera)
    float BobFrequency = 5.0f; // 흔들림 빈도 (발걸음 속도)

    UPROPERTY(EditDefaultsOnly, Category = Camera)
    float BobAmplitude = 2.0f;  // 흔들림 강도 (위아래 폭)

    UPROPERTY(EditDefaultsOnly, Category = Camera)
    float BobHorizontalAmplitude = 1.0f; // 흔들림 강도 (좌우 폭)


protected:
    // 이동 처리 함수
	void Move(const FInputActionValue& Value);

	// 시점 처리 함수
	void Look(const FInputActionValue& Value);

    // 달리기 함수
	void StartRunning(const FInputActionValue& Value);

    // 달리기 상태 감시 함수
	void CheckRunState();

	// 크로우치 토글 함수
	void ToggleCrouch(const FInputActionValue& Value);
};
