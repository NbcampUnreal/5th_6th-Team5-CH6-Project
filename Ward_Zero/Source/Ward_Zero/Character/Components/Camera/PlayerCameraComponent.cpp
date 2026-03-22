#include "Character/Components/Camera/PlayerCameraComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Data/Camera/CameraData.h"
#include "Character/Components/Interaction/InteractionComponent.h"

UPlayerCameraComponent::UPlayerCameraComponent() { PrimaryComponentTick.bCanEverTick = true; }

void UPlayerCameraComponent::Initialize(USpringArmComponent* InBoom, UCameraComponent* InCamera, UCameraData* InData)
{
    OwnerCharacter = Cast<APrototypeCharacter>(GetOwner());
    CameraBoom = InBoom;
    MainCamera = InCamera;
    CameraData = InData;

    // 초기화 시점에 컴포넌트 캐싱
    if (OwnerCharacter)
    {
        CachedCombatComp = OwnerCharacter->FindComponentByClass<UPlayerCombatComponent>();
    }

    if (CameraBoom)
    {
        OriginalSocketOffset = CameraBoom->SocketOffset;
        OriginalTargetOffset = CameraBoom->TargetOffset;
    }
}

void UPlayerCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateCamera(DeltaTime);
}

void UPlayerCameraComponent::UpdateCamera(float DeltaTime)
{
    if (!OwnerCharacter || !CameraBoom || !MainCamera || !CameraData || !CachedCombatComp) return;

    UPlayerCombatComponent* Combat = CachedCombatComp;

    float Speed = OwnerCharacter->GetVelocity().Size();

    bool bIsCrouchedNow = OwnerCharacter->bIsCrouched;
    if (bIsCrouchedNow != bWasCrouched)
    {
        float CapsuleHeightDiff = 45.0f;

        if (bIsCrouchedNow)
        {
            CameraBoom->SocketOffset.Z += CapsuleHeightDiff;
        }
        else
        {
            CameraBoom->SocketOffset.Z -= CapsuleHeightDiff;
        }
        bWasCrouched = bIsCrouchedNow;
    }

    // 목표 수치 초기화 
    float TargetArmLength = CameraData->DefaultArmLength;
    float TargetFOV = CameraData->DefaultFOV;
    FVector TargetSocketOffset = CameraData->DefaultSocketOffset;

    // 조준 상태 판별
    if (Combat->IsAiming())
    {
        TargetArmLength = CameraData->AimArmLength;
        TargetFOV = CameraData->AimFOV;

        // 무기별 조준 오프셋 적용
        if (OwnerCharacter->bIsCrouched)
        {
            TargetSocketOffset = (Combat->GetCurrentWeaponIndex() == 2) ? CameraData->CrouchedSMGAimSocketOffset : CameraData->CrouchedPistolAimSocketOffset;

            if (Combat->GetCurrentWeaponIndex() == 1 && Speed > 10.0f)
            {
                TargetSocketOffset.Z += CameraData->CrouchedWalkAimZOffset;
            }
        }
        else
        {
            TargetSocketOffset = (Combat->GetCurrentWeaponIndex() == 2) ? CameraData->SMGAimSocketOffset : CameraData->PistolAimSocketOffset;
        }

        if (!Combat->IsRecoiling())
        {
            BobTime += DeltaTime;
        }

        float CurrentSway = (Speed > 10.f) ? CameraData->WalkSwayIntensity : CameraData->BreathSwayIntensity;

        float SwayPitch = FMath::Sin(BobTime * 2.0f) * CurrentSway * 0.05f;
        float SwayYaw = FMath::Cos(BobTime * 1.0f) * CurrentSway * 0.05f;

        FRotator NewSwayRot = FRotator(SwayPitch, SwayYaw, 0.0f);
        FRotator DeltaSway = NewSwayRot - LastSwayRot;
        LastSwayRot = NewSwayRot;

        if (AController* Controller = OwnerCharacter->GetController())
        {
            Controller->SetControlRotation(Controller->GetControlRotation() + DeltaSway);
        }
    }
    else
    {
        if (!LastSwayRot.IsNearlyZero())
        {
            if (AController* Controller = OwnerCharacter->GetController())
            {
                Controller->SetControlRotation(Controller->GetControlRotation() - LastSwayRot);
            }
            LastSwayRot = FRotator::ZeroRotator;
        }

        // 비조준 상태
        if (OwnerCharacter->bIsCrouched)
        {
            TargetArmLength = CameraData->CrouchedArmLength;

            float VerticalBase = (Speed > 10.f && Combat->GetCurrentWeaponIndex() != 2) ? CameraData->CrouchedWalkCameraHeight : CameraData->CrouchedCameraHeight;
            TargetSocketOffset.Z = VerticalBase + 20.0f;
        }

        // 이동 시 흔들림 (Bobbing) - 비조준 
        if (Speed > 10.f && OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())
        {
            float Freq = CameraData->BobFrequency;
            float Amp = CameraData->BobAmplitude;
            float SpeedFactor = 1.0f;

            if (OwnerCharacter->bIsCrouched) { Freq *= 0.8f; Amp *= 0.5f; SpeedFactor = Speed / 150.f; }
            else if (OwnerCharacter->GetIsRunning())
            {
                Freq *= CameraData->RunFrequencyAmplify;
                Amp *= CameraData->RunAmplitudeAmplify;
                SpeedFactor = 1.0f;
            }
            else
            {
                SpeedFactor = Speed / 150.f;
            }

            BobTime += DeltaTime * SpeedFactor * Freq;
            TargetSocketOffset.Z += FMath::Sin(BobTime) * Amp;

            float HorizontalAmp = CameraData->BobHorizontalAmplitude * (OwnerCharacter->GetIsRunning() ? 1.5f : 1.0f);
            TargetSocketOffset.Y += FMath::Cos(BobTime * 0.5f) * HorizontalAmp;
        }
        else { BobTime = FMath::FInterpTo(BobTime, 0.0f, DeltaTime, 5.0f); }
    }

    // 부드러운 보간 적용 
    float Interp = CameraData->InterpSpeed;
    CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, Interp);
    MainCamera->FieldOfView = FMath::FInterpTo(MainCamera->FieldOfView, TargetFOV, DeltaTime, Interp);
    CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, Interp);
}