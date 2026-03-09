#include "Character/Components/Camera/PlayerCameraComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Data/Camera/CameraData.h"

UPlayerCameraComponent::UPlayerCameraComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UPlayerCameraComponent::Initialize(USpringArmComponent* InBoom, UCameraComponent* InCamera, UCameraData* InData)
{
    OwnerCharacter = Cast<APrototypeCharacter>(GetOwner());
    CameraBoom = InBoom;
    MainCamera = InCamera;
    CameraData = InData;

    if (CameraBoom)
    {
        OriginalSocketOffset = CameraBoom->SocketOffset;
        OriginalTargetOffset = CameraBoom->TargetOffset;
    }
}

void UPlayerCameraComponent::UpdateCamera(float DeltaTime)
{
    if (!OwnerCharacter || !CameraBoom || !MainCamera || !CameraData) return;

    UPlayerCombatComponent* Combat = OwnerCharacter->FindComponentByClass<UPlayerCombatComponent>();
    if (!Combat) return;

    float Speed = OwnerCharacter->GetVelocity().Size();

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
        TargetSocketOffset = (Combat->GetCurrentWeaponIndex() == 2) ?
            CameraData->SMGAimSocketOffset : CameraData->PistolAimSocketOffset;

        BobTime = 0.0f;
    }
    else
    {
        // 비조준 상태
        if (OwnerCharacter->bIsCrouched)
        {
            TargetArmLength = CameraData->CrouchedArmLength;

            float VerticalBase = (Speed > 10.f) ? CameraData->CrouchedWalkCameraHeight : CameraData->CrouchedCameraHeight;
            TargetSocketOffset.Z = VerticalBase + 20.0f;
        }

        // 이동 시 흔들림 (Bobbing) - 비조준 
        if (Speed > 10.f && OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())
        {
            float Freq = CameraData->BobFrequency;
            float Amp = CameraData->BobAmplitude;

            if (OwnerCharacter->bIsCrouched) { Freq *= 0.8f; Amp *= 0.5f; }

            BobTime += DeltaTime * (Speed / 150.f) * Freq;
            TargetSocketOffset.Z += FMath::Sin(BobTime) * Amp;
            TargetSocketOffset.Y += FMath::Cos(BobTime * 0.5f) * CameraData->BobHorizontalAmplitude;
        }
        else { BobTime = FMath::FInterpTo(BobTime, 0.0f, DeltaTime, 5.0f); }
    }

    // 부드러운 보간 적용 
    float Interp = CameraData->InterpSpeed;
    CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, Interp);
    MainCamera->FieldOfView = FMath::FInterpTo(MainCamera->FieldOfView, TargetFOV, DeltaTime, Interp);
    CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, Interp);
}