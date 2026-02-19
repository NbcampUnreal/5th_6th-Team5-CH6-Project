#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Character/Components/PlayerStatusComponent.h" 
#include "Character/Components/PlayerCombatComponent.h" 
#include "Character/Animation/Interface/PlayerAnimInterface.h"
#include "PrototypeCharacter.generated.h"

// 전방 선언 (Forward Declarations)
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ALadder;
class UCameraShakeBase;

UENUM(BlueprintType)
enum class EPlayerHitDirection : uint8
{
    Front,
    Back,
    Left,
    Right
};

UENUM(BlueprintType)
enum class EQuickTurnType : uint8
{
    None,
    Unarmed_180,
    Unarmed_L90,
    Unarmed_R90,
    Pistol_180,
    Pistol_L90,
    Pistol_R90
};

UCLASS()
class WARD_ZERO_API APrototypeCharacter : public ACharacter, public IPlayerAnimInterface
{
    GENERATED_BODY()

public:
    APrototypeCharacter();

protected:
#pragma region 기본 캐릭터 함수 (Character Overrides)
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
#pragma endregion 

public:
#pragma region 인터페이스 구현 (IPlayerAnimInterface)
    virtual bool GetIsRunning() const override;
    virtual bool GetIsPistolEquipped() const override;
    virtual bool GetIsCrouching() const override;
    virtual bool GetIsGround() const override;
    virtual bool GetIsQuickTurning() const override;
    virtual int32 GetTurnIndex() const override;
    virtual bool IsEquipping() const override; 
    virtual bool GetIsAiming() const override;
    virtual FVector GetHandIKTargetLoc() const override;
    virtual void SetIsQuickTurning(bool bIsTurning) override; 
    virtual bool GetIsClimbing() const override; 
#pragma endregion

protected:
#pragma region 컴포넌트 (Components)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Status")
    UPlayerStatusComponent* StatusComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Combat")
    UPlayerCombatComponent* CombatComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Camera")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Camera")
    UCameraComponent* MainCamera;
#pragma endregion 

#pragma region 입력 시스템 (Input System)
    // 입력 매핑 & 액션
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, Category = "Input|Actions") UInputAction* MoveAction;
    UPROPERTY(EditAnywhere, Category = "Input|Actions") UInputAction* LookAction;
    UPROPERTY(EditAnywhere, Category = "Input|Actions") UInputAction* RunAction;
    UPROPERTY(EditAnywhere, Category = "Input|Actions") UInputAction* CrouchAction;
    UPROPERTY(EditAnywhere, Category = "Input|Actions") UInputAction* InteractAction;
    UPROPERTY(EditAnywhere, Category = "Input|Actions") UInputAction* EquipAction;
    UPROPERTY(EditAnywhere, Category = "Input|Actions") UInputAction* AimAction;
    UPROPERTY(EditAnywhere, Category = "Input|Actions") UInputAction* FireAction;
    UPROPERTY(EditAnywhere, Category = "Input|Actions") UInputAction* QuickTurnAction;
    UPROPERTY(EditAnywhere, Category = "Input|Actions") UInputAction* ReloadAction;

    // 입력 처리 바인딩 함수
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartRunning(const FInputActionValue& Value);
    void EndRunning(const FInputActionValue& Value);
    void ToggleCrouch(const FInputActionValue& Value);
    void Interact(const FInputActionValue& Value);
    void ToggleEquip(const FInputActionValue& Value);
    void StartAiming(const FInputActionValue& Value);
    void StopAiming(const FInputActionValue& Value);
    void Fire(const FInputActionValue& Value);
    void Reload(const FInputActionValue& Value);
#pragma endregion 

#pragma region 이동 및 카메라 설정 (Movement & Camera Config)
    // 이동 속도 설정
    UPROPERTY(EditDefaultsOnly, Category = "Movement|Speed") float WalkSpeed = 200.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Movement|Speed") float RunSpeed = 450.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Movement|Speed") float ClimbSpeed = 150.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Movement|Speed") float CrouchMovementSpeed = 150.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Movement|Turn") float WalkTurnRate = 2.5f;

    // 카메라 위치 & 밥(Bob) 설정
    UPROPERTY(EditDefaultsOnly, Category = "Camera|Base") float CrouchedArmLength = 130.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Camera|Base") float CrouchedCameraHeight = 40.0f;
    UPROPERTY(EditAnywhere, Category = "Camera|Bob") float BobFrequency = 12.0f;
    UPROPERTY(EditAnywhere, Category = "Camera|Bob") float BobAmplitude = 2.0f;
    UPROPERTY(EditAnywhere, Category = "Camera|Bob") float BobHorizontalAmplitude = 1.0f;

    // 조준 설정
    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim") float AimLookSensitivity = 0.5f;
    UPROPERTY(EditAnywhere, Category = "Camera|Aim") float AimArmLength = 40.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim") float AimFOV = 50.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Aim") FVector AimSocketOffset = FVector(-20.0f, 30.0f, 20.0f);
    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim") float AimInterpSpeed = 20.0f;
#pragma endregion 

public:
#pragma region 퀵 턴 시스템 (Quick Turn System)
    UPROPERTY(BlueprintReadOnly, Category = "Turn")
    EQuickTurnType QuickTurnType = EQuickTurnType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Turn")
    int32 TurnIndex = 0;

    UFUNCTION(BlueprintCallable, Category = "Turn")
    void StartQuickTurn(float TargetYawDelta);

    UFUNCTION(BlueprintCallable, Category = "Turn")
    void StopQuickTurn();

    void PerformQuickTurn180();
    void PerformQuickTurn90(float Angle);

protected:
    void ProcessMovementTurn(FVector2D MovementVector);

    UPROPERTY(EditDefaultsOnly, Category = "Turn|Duration") 
    float Duration180 = 0.6f;

    UPROPERTY(EditDefaultsOnly, Category = "Turn|Duration") 
    float Duration90 = 0.4f;
#pragma endregion 

public:
#pragma region 등반 시스템 (Climbing System)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing")
    bool bIsClimbing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Climbing")
    TObjectPtr<ALadder> CurrentLadder = nullptr;

    UFUNCTION(BlueprintCallable)
    void StartClimbing(ALadder* Ladder);

    UFUNCTION(BlueprintCallable)
    void StopClimbing();
#pragma endregion

protected:
#pragma region 애니메이션 및 몽타주 (Animation & Montages)
    // 애니메이션 레이어 링크
    UPROPERTY(EditAnywhere, Category = "Animations|Layers") TSubclassOf<class UAnimInstance> UnarmedLayerClass;
    UPROPERTY(EditAnywhere, Category = "Animations|Layers") TSubclassOf<class UAnimInstance> PistolLayerClass;

    // 액션 몽타주
    UPROPERTY(EditAnywhere, Category = "Animations|Action") UAnimMontage* EquipMontage;
    UPROPERTY(EditAnywhere, Category = "Animations|Action") UAnimMontage* UnEquipMontage;
    UPROPERTY(EditAnywhere, Category = "Animations|Action") UAnimMontage* FireMontage;

    // 피격 몽타주
    UPROPERTY(EditDefaultsOnly, Category = "Animations|Hit") UAnimMontage* HitMontage_Front;
    UPROPERTY(EditDefaultsOnly, Category = "Animations|Hit") UAnimMontage* HitMontage_Back;
    UPROPERTY(EditDefaultsOnly, Category = "Animations|Hit") UAnimMontage* HitMontage_Right;
    UPROPERTY(EditDefaultsOnly, Category = "Animations|Hit") UAnimMontage* HitMontage_Left;

    // 사망 몽타주
    UPROPERTY(EditDefaultsOnly, Category = "Animations|Death") UAnimMontage* DeathMontage_Front;
    UPROPERTY(EditDefaultsOnly, Category = "Animations|Death") UAnimMontage* DeathMontage_Back;
    UPROPERTY(EditDefaultsOnly, Category = "Animations|Death") UAnimMontage* DeathMontage_Left;
    UPROPERTY(EditDefaultsOnly, Category = "Animations|Death") UAnimMontage* DeathMontage_Right;

    // 카메라 쉐이크
    UPROPERTY(EditAnywhere, Category = "Animations|Effects") TSubclassOf<UCameraShakeBase> FireCameraShake;

    // 리액션 처리 함수
    void PlayHitReaction(const FVector& ToAttackerDir);
    void PlayDeathReaction(const FVector& ToAttackerDir);
    EPlayerHitDirection GetHitDirection(const FVector& ToAttackerDir);

    UFUNCTION()
    void OnDeath(); // StatusComponent가 죽었다고 하면 실행
#pragma endregion 

private:
#pragma region 내부 상태 및 계산 변수 (Internal State & Data)
    // 달리기 상태
    bool bIsRunning = false;
    void CheckRunState();

    // 턴 시스템 계산용 
    UPROPERTY()
    bool bIsQuickTurning = false;
    float TurnAlpha = 0.0f;
    float TurnDuration = 0.0f;     
    float TurnStartYaw = 0.0f;     
    float TurnYawDelta = 0.0f;     
    float ControlStartYaw = 0.0f;

<<<<<<< Updated upstream
    float TurnStartYaw = 0.0f;     // 회전 시작 당시의 Yaw
    float TurnYawDelta = 0.0f;     // 회전해야 할 총 각도 (예: 90, 180, -90)

    float ControlStartYaw;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    int32 TurnIndex = 0;

    // 퀵 턴 키 바인딩 함수
    void PerformQuickTurn180();

    void PerformQuickTurn90(float Angle);

 protected:
     // 이동 입력 시 각도 체크를 위한 변수
     void ProcessMovementTurn(FVector2D MovementVector);

     // 각 애니메이션별 재생 시간
     UPROPERTY(EditDefaultsOnly, Category = "Turn|Duration") 
     float Duration180 = 0.6f;

     UPROPERTY(EditDefaultsOnly, Category = "Turn|Duration") 
     float Duration90 = 0.4f;

public:
    // 애니메이션 레이어 링크용 클래스
    UPROPERTY(EditAnywhere, Category = "Animations")
    TSubclassOf<class UAnimInstance> UnarmedLayerClass;

    UPROPERTY(EditAnywhere, Category = "Animations")
    TSubclassOf<class UAnimInstance> PistolLayerClass;

public:
    // 인터페이스 함수 오버라이드 선언
    virtual bool GetIsRunning() const override;
    virtual bool GetIsPistolEquipped() const override;
    virtual bool GetIsCrouching() const override;
    virtual bool GetIsGround() const override;
    virtual bool GetIsQuickTurning() const override;
    virtual int32 GetTurnIndex() const override;
    virtual  bool IsEquipping() const override; 
    virtual bool GetIsAiming() const override;
    FVector GetHandIKTargetLoc() const override;
    virtual void SetIsQuickTurning(bool bIsTurning) override; 
    virtual bool GetIsClimbing() const override; 
    virtual UStaticMeshComponent* GetEquippedWeaponMesh() override;
=======
    // 카메라 및 조준 복원용 변수
    float StandingArmLength = 180.0f;
    float StandingCameraHeight = 45.0f;
    float CurrentBaseCameraZ = 45.0f;
    float BobTime = 0.0f;
    
    float OriginalArmLength = 180.0f;
    FVector OriginalSocketOffset;
    FVector OriginalTargetOffset;
    float OriginalFOV = 60.0f;
#pragma endregion 
>>>>>>> Stashed changes
};