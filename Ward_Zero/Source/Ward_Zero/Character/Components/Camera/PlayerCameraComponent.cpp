#include "Character/Components/Camera/PlayerCameraComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Data/Camera/CameraData.h"
#include "Character/Components/Interaction/InteractionComponent.h"
#include "Weapon/Weapon.h"
#include "Components/CapsuleComponent.h"

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
    APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
    
    bool bIsInVentNow = OwnerCharacter->bIsInVent;

    // 환풍구 진입 / 탈출 
    if (bIsInVentNow && !bIsInVent) // 진입
    {
        CameraBoom->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        CameraBoom->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("head"));

        CameraBoom->bDoCollisionTest = true;
        CameraBoom->TargetArmLength = 0.0f;
        CameraBoom->SocketOffset = FVector::ZeroVector;
        CameraBoom->TargetOffset = FVector::ZeroVector;
        CameraBoom->SetRelativeLocation(FVector::ZeroVector);
        CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);

        // 캐릭터 메쉬 전체 숨김
        TArray<UMeshComponent*> AllMeshes;
        OwnerCharacter->GetComponents<UMeshComponent>(AllMeshes);
        for (UMeshComponent* MeshComp : AllMeshes)
        {
            MeshComp->SetOwnerNoSee(true);
        }

        if (PC && PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->ViewPitchMin = -50.0f; 
            PC->PlayerCameraManager->ViewPitchMax = 7.0f;  
        }
        bIsInVent = true;
    }
    else if (!bIsInVentNow && bIsInVent) // 탈출
    {
        CameraBoom->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        CameraBoom->AttachToComponent(OwnerCharacter->GetCapsuleComponent(), FAttachmentTransformRules::KeepWorldTransform);

        CameraBoom->bDoCollisionTest = true;
        CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
        CameraBoom->SetRelativeRotation(FRotator::ZeroRotator);
        CameraBoom->SocketOffset = OriginalSocketOffset;
        CameraBoom->TargetOffset = OriginalTargetOffset;

        // 메쉬 복구
        TArray<UMeshComponent*> MeshComps;
        OwnerCharacter->GetComponents<UMeshComponent>(MeshComps);
        for (UMeshComponent* Mesh : MeshComps)
        {
            Mesh->SetOwnerNoSee(false);
        }
        if (PC && PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->ViewPitchMin = -89.0f;
            PC->PlayerCameraManager->ViewPitchMax = 89.0f;
        }
        bIsInVent = false;
    }

    // 카메라 목표 수치 및 업데이트
    float TargetArmLength = CameraData->DefaultArmLength;
    float TargetFOV = CameraData->DefaultFOV;
    FVector TargetSocketOffset = CameraData->DefaultSocketOffset;

    bool bIsFPSMode = OwnerCharacter->bIsCrouched && OwnerCharacter->bIsInVent;

    if (bIsFPSMode)
    {
        TargetArmLength = 0.0f;
        TargetSocketOffset = FVector(25.0f, 0.0f, 10.0f);
        TargetFOV = 90.0f;
    }
    else if (Combat->IsAiming())
    {
        TargetArmLength = CameraData->AimArmLength;
        TargetFOV = CameraData->AimFOV;
        TargetSocketOffset = (Combat->GetCurrentWeaponIndex() == 2) ? CameraData->SMGAimSocketOffset : CameraData->PistolAimSocketOffset;

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

        if (OwnerCharacter->bIsCrouched)
        {
            TargetArmLength = CameraData->CrouchedArmLength;
            TargetSocketOffset = FVector(0.0f, 0.0f, CameraData->CrouchedCameraHeight);
        }

        // Bobbing
        if (Speed > 10.f && OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())
        {
            float Freq = CameraData->BobFrequency;
            float Amp = CameraData->BobAmplitude;
            float SpeedFactor = (OwnerCharacter->bIsCrouched) ? Speed / 150.f : Speed / 150.f;

            if (!OwnerCharacter->bIsCrouched && OwnerCharacter->GetIsRunning())
            {
                Freq *= CameraData->RunFrequencyAmplify;
                Amp *= CameraData->RunAmplitudeAmplify;
                SpeedFactor = 1.0f;
            }

            BobTime += DeltaTime * SpeedFactor * (OwnerCharacter->bIsCrouched ? Freq * 0.8f : Freq);
            TargetSocketOffset.Z += FMath::Sin(BobTime) * (OwnerCharacter->bIsCrouched ? Amp * 0.5f : Amp);
            TargetSocketOffset.Y += FMath::Cos(BobTime * 0.5f) * (CameraData->BobHorizontalAmplitude * (OwnerCharacter->GetIsRunning() ? 1.5f : 1.0f));
        }
        else { BobTime = FMath::FInterpTo(BobTime, 0.0f, DeltaTime, 5.0f); }
    }

    // 경사로 오프셋
    float TargetSlopeOffset = 0.0f;
    if (!bIsFPSMode && !Combat->IsAiming() && Speed > 10.0f)
    {
        if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
        {
            if (MovementComp->IsMovingOnGround())
            {
                FVector FloorNormal = MovementComp->CurrentFloor.HitResult.ImpactNormal;
                FVector CameraForward = MainCamera->GetForwardVector();
                CameraForward.Z = 0.0f;
                CameraForward.Normalize();

                float SlopeDot = FVector::DotProduct(FloorNormal, CameraForward);
                if (SlopeDot < -0.05f)
                {
                    TargetSlopeOffset = FMath::Clamp(SlopeDot * 150.0f, -40.0f, 0.0f);
                }
            }
        }
    }

    CurrentSlopeOffset = FMath::FInterpTo(CurrentSlopeOffset, TargetSlopeOffset, DeltaTime, 4.0f);
    TargetSocketOffset.Z += CurrentSlopeOffset;

    float Interp = CameraData->InterpSpeed;
    CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, Interp);
    MainCamera->FieldOfView = FMath::FInterpTo(MainCamera->FieldOfView, TargetFOV, DeltaTime, Interp);
    CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaTime, Interp);

    if (MainCamera && OwnerCharacter)
    {
        float DistanceToCamera = FVector::Dist(MainCamera->GetComponentLocation(), OwnerCharacter->GetActorLocation());

        bool bTooClose = DistanceToCamera < 75.0f;

        if (OwnerCharacter->GetMesh())
        {
            OwnerCharacter->GetMesh()->SetOwnerNoSee(bTooClose);
        }
        if (Combat)
        {
            AWeapon* CurrentWeapon = Combat->GetEquippedWeapon();
            if (CurrentWeapon && CurrentWeapon->WeaponMesh)
            {
                CurrentWeapon->WeaponMesh->SetOwnerNoSee(bTooClose);
            }
        }
    }
}