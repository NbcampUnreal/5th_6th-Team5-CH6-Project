#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Character/Enum/CharacterType.h"
#include "Character/Animation/Interface/PlayerAnimInterface.h"
#include "Components/TimelineComponent.h"
#include "PrototypeCharacter.generated.h"

UENUM(BlueprintType)
enum class EWeaponLayerType : uint8 { Unarmed, Pistol, SMG };

UCLASS()
class WARD_ZERO_API APrototypeCharacter : public ACharacter, public IPlayerAnimInterface
{
	GENERATED_BODY()

public:
	APrototypeCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:
#pragma region 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") class UPlayerStatusComponent* StatusComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") class UPlayerCombatComponent* CombatComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") class UInteractionComponent* InteractionComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") class UQuickTurnComponent* QuickTurnComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") class UFlashlightComponent* FlashLightComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") class UFootstepComponent* FootstepComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") class UPlayerCameraComponent* CustomCameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera") class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera") class UCameraComponent* MainCamera;


	UPROPERTY(EditDefaultsOnly, Category = "Camera|Effects")
	TSubclassOf<class UCameraShakeBase> HitCameraShakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UHealthVignetteWidget> HealthVignetteClass;

	UPROPERTY()
	UHealthVignetteWidget* HealthVignetteWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UMotionWarpingComponent* MotionWarpingComp;
#pragma endregion

#pragma region 데이터 에셋
	UPROPERTY(EditDefaultsOnly, Category = "DataAsset") class UCharacterMovementData* MovementData;
	UPROPERTY(EditDefaultsOnly, Category = "DataAsset") class UCharacterStatusData* StatusData;
	UPROPERTY(EditDefaultsOnly, Category = "DataAsset") class UCharacterAnimData* AnimData;
	UPROPERTY(EditDefaultsOnly, Category = "DataAsset") class UCharacterCombatData* CombatData;
	UPROPERTY(EditDefaultsOnly, Category = "DataAsset") class UCameraData* CameraConfig;
#pragma endregion

#pragma region 입력 액션
	UPROPERTY(EditAnywhere, Category = "Input") class UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* RunAction;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* InteractAction;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* EquipAction;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* AimAction;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* FireAction;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* QuickTurnAction;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* ReloadAction;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* FlashlightAction;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* EquipSlot1Action;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* EquipSlot2Action;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* HealAction;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* PauseAction;
#pragma endregion

#pragma region 애니메이션 레이어
	UPROPERTY(EditAnywhere, Category = "Animation") TSubclassOf<UAnimInstance> UnarmedLayer;
	UPROPERTY(EditAnywhere, Category = "Animation") TSubclassOf<UAnimInstance> PistolLayer;
	UPROPERTY(EditAnywhere, Category = "Animation") TSubclassOf<UAnimInstance> SMGLayer;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	EWeaponLayerType CurrentLayerType = EWeaponLayerType::Unarmed;
#pragma endregion

#pragma region IPlayerAnimInterface 구현
	virtual bool GetIsRunning() const override { return bIsRunning; }
	virtual bool GetIsPistolEquipped() const override;
	virtual bool GetIsCrouching() const override { return bIsCrouched; }
	virtual bool GetIsGround() const override;
	virtual bool GetIsQuickTurning() const override;
	virtual int32 GetTurnIndex() const override;
	virtual bool IsEquipping() const override;
	virtual FVector GetHandIKTargetLoc() const override;
	virtual bool GetIsAiming() const override;
	virtual void SetIsQuickTurning(bool bIsTurning) override;
	virtual USkeletalMeshComponent* GetEquippedWeaponMesh() override;
	virtual class AWeapon* GetEquippedWeapon() override;
	virtual bool GetIsReloading() const override;
	virtual bool GetIsUseFlashLight() const override;
	virtual bool GetIsSMGEquipped() const override;
	virtual int32 GetCurrentWeaponIndex() const override;
	virtual float GetAimPitch() const override;
	virtual float GetAimYaw() const override;
	virtual bool IsFiring() const override;
	virtual float GetCurrSpread() const override;
	virtual UPlayerCombatComponent* GetCombatComp() const override;
	virtual bool GetbIsWeaponDrawn() const override; 
	virtual bool GetIsInjured() const override;
	virtual void ExecuteHealPoint() override;
	virtual bool GetIsInteracting() const override;
	virtual bool GetIsInVent() const override;
	virtual bool GetIsUseHeal() const override;
#pragma endregion

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
	void StopFire(const FInputActionValue& Value);
	void Reload(const FInputActionValue& Value);
	void ToggleFlashLight(const FInputActionValue& Value);
	void SelectWeapon1(const FInputActionValue& Value);
	void SelectWeapon2(const FInputActionValue& Value);
	void TogglePauseMenu(const FInputActionValue& Value);

	void CheckRunState();
	void PerformQuickTurn180();
	void OnDeath();

	UFUNCTION(BlueprintCallable)
	void PlayFootstepSound(FName FootBoneName);

protected:
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

public:
	EPlayerHitDirection GetHitDirection(const FVector& ToAttackerDir);
	void PlayHitReaction(const FVector& ToAttackerDir);
	void PlayDeathReaction(const FVector& ToAttackerDir);

private:
	bool bIsRunning = false;
	float BobTime = 0.0f; // 카메라 밥 계산용

	// 원본 백업 변수들
	float OriginalArmLength = 180.0f;
	FVector OriginalSocketOffset = FVector(0.0f, 35.0f, 10.0f);
	float OriginalFOV = 60.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float AimLookSensitivity = 0.5f;

	void UpdateBodyRotation(float DeltaTime); // 캐릭터 몸체 회전 함수 

public:
	bool bHasKeyCard = false;

	bool HasKeyCard() const { return bHasKeyCard; }
	void GiveKeyCard() { bHasKeyCard = true; }

public:
	UFUNCTION(BlueprintCallable)
	void Revive();

	void StartHeal();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void OnHealPoint();

public:
	UFUNCTION(BlueprintCallable)
	void SpawnHealItemVisual();

	UFUNCTION(BlueprintCallable)
	void DestroyHealItemVisual();

	UFUNCTION(BlueprintCallable)
	void PopHealItemCap();

public:

	// 애니메이션 데이터 에셋에 픽업 몽타주가 있다고 가정 (AnimData에 추가 필요)
	void PlayPickupAnimation(AActor* TargetItem);


	// 노티파이에서 호출할 함수
	UFUNCTION(BlueprintCallable)
	void AttachInteractingItem();

	UFUNCTION(BlueprintCallable)
	void ConsumeInteractingItem();

public:
	bool bIsInteractingDoor = false;

	UPROPERTY()
	AActor* LastInteractedDoorActor = nullptr;

	float LastDoorInteractTime = 0.0f;

	FVector CurrentPickupLocation;
private:
	AActor* FindClosestInteractable();
	void HandleDoorInteraction(AActor* DoorActor);
	void HandleItemInteraction(AActor* ItemActor);
	void HandleLeverInteraction(AActor* LeverActor);

	void SwitchWeaponByIndex(int32 WeaponIndex);

	// 피격 시 공격자 방향 계산 분리
	FVector GetAttackerDirection(AController* EventInstigator, AActor* DamageCauser);

public:
	// 레버 손잡이 위치를 추적할 IK 타겟
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Animation|IK")
	FVector InteractionIKTargetLoc;

	// 레버 IK 적용 강도 (0.0 ~ 1.0)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Animation|IK")
	float InteractionIKAlpha = 0.0f;

public:
	UFUNCTION()void SetDoorPasscode(int32 Passcode);

	void AbortAllActions();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bIsInVent = false;

	UFUNCTION()
	void SetIsVent(bool IsVent) { bIsInVent = IsVent;}

	int32 VentOverlapCount = 0;

	void UpdateVentState();
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	//UPROPERTY()
	//FTimeline ReviveCameraTimeline;

	//UPROPERTY(EditAnywhere, Category = "Camera")
	//UCurveFloat* ReviveCameraCurve; // 0초에서 -60(아래), 끝날 때 0(정면)으로 변하는 커브 설정

	/*UFUNCTION()
	void UpdateReviveCamera(float Value);*/

	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> IdleShakeClass;

	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> WalkShakeClass;

	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> RunShakeClass;

	UPROPERTY()
	UCameraShakeBase* CurrentCameraShake;

	// ABP Base에서 State(Idle/Walk/Run) 호출할 노티파이 이벤트
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ChangeLocomotionCameraShake(int32 StateIndex);

	bool bIsUseHeal = false;

private:
	bool bWasWeaponDrawnBeforeHeal = false;

	UFUNCTION()
	void OnHealMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY()
	bool bWasInteractingOnHit = false;
};