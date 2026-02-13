#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Character/Components/PlayerStatusComponent.h" 
#include "Character/Components/PlayerCombatComponent.h" 
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

UCLASS()
class WARD_ZERO_API APrototypeCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APrototypeCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    UStaticMeshComponent* PistolMesh;
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
#pragma endregion 

#pragma region Montage 
    // 몽타주 
    UPROPERTY(EditAnywhere, Category = "Montage") UAnimMontage* EquipMontage;
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
    UPROPERTY(EditDefaultsOnly, Category = "Camera") float BobFrequency = 5.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Camera") float BobAmplitude = 2.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Camera") float BobHorizontalAmplitude = 1.0f;

    // 조준 시 줌 설정
    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim") float AimArmLength = 80.0f;
    UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim") float AimFOV = 45.0f;

    FVector DefaultTargetOffset;

    //조준 해제 시 복원용 초기값 저장 변수 
    float OriginalArmLength = 180.0f;
    FVector OriginalSocketOffset;
    FVector OriginalTargetOffset;
    float OriginalFOV = 60.0f;
    bool bOriginalUsePawnControlRotation = true;
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
    void StartRunning(const FInputActionValue& Value);
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
};