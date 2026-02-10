#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "PrototypeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ALadder;

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* PistolMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Effects")
    UNiagaraSystem* MuzzleFlash;//총구 화염 

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Effects")
    UNiagaraSystem* ImpactEffect;//피격 이펙트

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Effects")
    UNiagaraSystem* LaserSightSystem; //레이저 
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Effects")
    TSubclassOf<UCameraShakeBase> FireCameraShake;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* EquipAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* AimAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* FireAction;

private:

	bool bIsRunning = false;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WalkSpeed = 200.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RunSpeed = 250.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float ClimbSpeed = 150.0f;

    UPROPERTY(EditDefaultsOnly, Category = Movement)
    float CrouchMovementSpeed = 150.0f;

    UPROPERTY(EditDefaultsOnly, Category = Camera)
    float CrouchedArmLength = 130.0f;

    float StandingArmLength = 150.0f;

    UPROPERTY(EditDefaultsOnly, Category = Movement)
    float WalkTurnRate = 2.5f;

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

    void Interact(const FInputActionValue& Value);

    void ToggleEquip(const FInputActionValue& Value);

    void StartAiming(const FInputActionValue& Value);

    void StopAiming(const FInputActionValue& Value);

    void Fire(const FInputActionValue& Value);

public:
    //Climbing Var & Fuc 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Climbing")
    bool bIsClimbing = false; 

    UPROPERTY(BlueprintReadOnly, Category="Climbing")
    TObjectPtr<ALadder> CurrentLadder = nullptr; 

    UFUNCTION(BlueprintCallable)
    void StartClimbing(ALadder* Ladder);

    UFUNCTION(BlueprintCallable)
    void StopClimbing();

    //Turn Var 
    UPROPERTY(BlueprintReadWrite)
    bool bIsQuickTurning = false;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    bool bIsPistolEquipped = false;

    UPROPERTY(EditAnywhere, Category="Montage")
    TObjectPtr<UAnimMontage> EquipMontage;

    UPROPERTY(EditAnywhere, Category = "Montage")
    TObjectPtr<UAnimMontage> UnequipMontage;

    UPROPERTY(EditAnywhere, Category = "Montage")
    TObjectPtr<UAnimMontage> FireMontage;

public:
    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
    float AimArmLength = 80.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
    FVector AimSocketOffset = FVector(0.f, 0.f, 38.f);

    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
    FVector AimTargetOffset = FVector(0.0f, 15.f, 30.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
    float AimFOV = 55.0f; // 광각을 줄여 집중도 향상

    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
    float AimCameraLagSpeed = 15.0f; //조준 시 카메라 이동  

    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
    float AimLookSensitivity = 0.5f; //마우스 민감도 

    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    bool bIsAiming_Anim = false;

    UPROPERTY(BlueprintReadOnly, Category = "Pistol|Zoom")
    bool bIsAiming = false;

    UPROPERTY(BlueprintReadOnly, Category = "Pistol")
    FVector HandIKTargetLocation;

private:
    UPROPERTY()
    TObjectPtr<UNiagaraComponent> LaserSightComponent;

    void UpdateLaserSight();

public:
    UPROPERTY(BlueprintReadOnly, Category = "Pistol")
    FVector HandIKTargetLocations;  // 손 타겟 (기존)

    UPROPERTY(BlueprintReadOnly, Category = "Pistol")
    FVector ElbowIKTargetLocation;

    // [추가] 기본 상태의 TargetOffset
    FVector DefaultTargetOffset;
};
