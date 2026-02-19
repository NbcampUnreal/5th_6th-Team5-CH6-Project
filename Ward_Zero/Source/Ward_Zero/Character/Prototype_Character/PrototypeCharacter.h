#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Character/Components/PlayerStatusComponent.h" 
#include "Character/Components/PlayerCombatComponent.h" 
#include "Character/Animation/Interface/PlayerAnimInterface.h"
#include "PrototypeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ALadder;

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
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;   
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, AActor* DamageCauser) override;

#pragma region Components
protected:
    // 컴포넌트 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPlayerStatusComponent* StatusComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPlayerCombatComponent* CombatComponent;

    // 카메라 & 메쉬 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    UCameraComponent* MainCamera;
#pragma endregion 

#pragma region IA
    // 입력 (IMC & Actions)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, Category = Input) UInputAction* MoveAction;
    UPROPERTY(EditAnywhere, Category = Input) UInputAction* LookAction;
    UPROPERTY(EditAnywhere, Category = Input) UInputAction* RunAction;
    UPROPERTY(EditAnywhere, Category = Input) UInputAction* CrouchAction;
    UPROPERTY(EditAnywhere, Category = Input) UInputAction* InteractAction;
    UPROPERTY(EditAnywhere, Category = Input) UInputAction* EquipAction;
    UPROPERTY(EditAnywhere, Category = Input) UInputAction* AimAction;
    UPROPERTY(EditAnywhere, Category = Input) UInputAction* FireAction;
    UPROPERTY(EditAnywhere, Category = Input) UInputAction* QuickTurnAction;
    UPROPERTY(EditAnywhere, Category = Input) UInputAction* ReloadAction;
#pragma endregion 

#pragma region Montage 
    // 몽타주 
    UPROPERTY(EditAnywhere, Category = "Montage") UAnimMontage* EquipMontage;

    UPROPERTY(EditAnywhere, Category = "Montage") UAnimMontage* UnEquipMontage;
    UPROPERTY(EditAnywhere, Category = "Montage") UAnimMontage* FireMontage;

    // 피격 몽타주
    UPROPERTY(EditDefaultsOnly, Category = "Montage|Hit") UAnimMontage* HitMontage_Front;
    UPROPERTY(EditDefaultsOnly, Category = "Montage|Hit") UAnimMontage* HitMontage_Back;
    UPROPERTY(EditDefaultsOnly, Category = "Montage|Hit") UAnimMontage* HitMontage_Right;
    UPROPERTY(EditDefaultsOnly, Category = "Montage|Hit") UAnimMontage* HitMontage_Left;

    // 사망 몽타주
    UPROPERTY(EditDefaultsOnly, Category = "Montage|Death") UAnimMontage* DeathMontage_Front;
    UPROPERTY(EditDefaultsOnly, Category = "Montage|Death") UAnimMontage* DeathMontage_Back;
    UPROPERTY(EditDefaultsOnly, Category = "Montage|Death") UAnimMontage* DeathMontage_Left;
    UPROPERTY(EditDefaultsOnly, Category = "Montage|Death") UAnimMontage* DeathMontage_Right;

    // 이펙트
    UPROPERTY(EditAnywhere, Category = "Weapon|Effects")
    TSubclassOf<UCameraShakeBase> FireCameraShake;
#pragma endregion 

#pragma region Var 
protected:
    // 이동 속도 설정
    UPROPERTY(EditDefaultsOnly, Category = "Movement") float WalkSpeed = 200.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Movement") float RunSpeed = 450.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Movement") float ClimbSpeed = 150.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Movement") float CrouchMovementSpeed = 150.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Movement") float WalkTurnRate = 2.5f;

    // 조준 설정 
    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim") float AimLookSensitivity = 0.5f;

    // 카메라 위치 설정
    UPROPERTY(EditDefaultsOnly, Category = "Camera") float CrouchedArmLength = 130.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Camera") float CrouchedCameraHeight = 40.0f;

    // 카메라 밥(Bob) 관련
    float StandingArmLength = 180.0f;
    float StandingCameraHeight = 45.0f;
    float CurrentBaseCameraZ = 45.0f;

    // 카메라 밥(Bob) 관련
    float BobTime = 0.0f;
    UPROPERTY(EditAnywhere, Category = "Camera") float BobFrequency = 12.0f;
    UPROPERTY(EditAnywhere, Category = "Camera") float BobAmplitude = 2.0f;
    UPROPERTY(EditAnywhere, Category = "Camera") float BobHorizontalAmplitude = 1.0f;

    // 조준 시 줌 설정
    UPROPERTY(EditAnywhere, Category = "Camera|Aim") float AimArmLength = 40.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim") float AimFOV = 50.0f;

    //조준 해제 시 복원용 초기값 저장 변수 
    float OriginalArmLength = 180.0f;
    FVector OriginalSocketOffset;
    FVector OriginalTargetOffset;
    float OriginalFOV = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Aim")
    FVector AimSocketOffset = FVector(-20.0f, 30.0f, 20.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
    float AimInterpSpeed = 20.0f;
#pragma endregion 

#pragma region Climb 
public:
    // 등반 시스템
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbing")
    bool bIsClimbing = false;

    UPROPERTY(BlueprintReadOnly, Category = "Climbing")
    TObjectPtr<ALadder> CurrentLadder = nullptr;

    UFUNCTION(BlueprintCallable)
    void StartClimbing(ALadder* Ladder);

    UFUNCTION(BlueprintCallable)
    void StopClimbing();

    // 턴 시스템
    UPROPERTY(BlueprintReadWrite)
    bool bIsQuickTurning = false;

private:
    bool bIsRunning = false;
    void CheckRunState();
#pragma endregion 


#pragma region Func
protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
	void Reload(const FInputActionValue& Value);
    void StartRunning(const FInputActionValue& Value);
    void EndRunning(const FInputActionValue& Value);
    void ToggleCrouch(const FInputActionValue& Value);
    void Interact(const FInputActionValue& Value);

    // 전투
    void ToggleEquip(const FInputActionValue& Value);
    void StartAiming(const FInputActionValue& Value);
    void StopAiming(const FInputActionValue& Value);
    void Fire(const FInputActionValue& Value);

    //Event 
    UFUNCTION()
    void OnDeath(); // StatusComponent가 죽었다고 하면 실행
#pragma endregion 

#pragma region Montage Fuc 
private:
    void PlayHitReaction(const FVector& ToAttackerDir);
    void PlayDeathReaction(const FVector& ToAttackerDir);
    EPlayerHitDirection GetHitDirection(const FVector& ToAttackerDir);
#pragma endregion 

//QuickTurn 
public:
    UPROPERTY(BlueprintReadOnly, Category = "Turn")
    EQuickTurnType QuickTurnType = EQuickTurnType::None;

    UFUNCTION(BlueprintCallable, Category = "Turn")
    void StartQuickTurn(float TargetYawDelta);

    UFUNCTION(BlueprintCallable, Category = "Turn")
    void StopQuickTurn();
  
    float TurnAlpha = 0.0f;
    float TurnDuration = 0.0f;     // 애니메이션 길이에 맞춰 설정 

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
};