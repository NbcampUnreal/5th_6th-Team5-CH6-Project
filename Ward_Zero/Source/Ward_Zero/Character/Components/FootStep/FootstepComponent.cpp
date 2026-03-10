#include "Character/Components/FootStep/FootstepComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"
#include "Character/Noise/NoiseFucLibrary/PlayerNoise.h"
#include "Character/Data/FootStep/FootstepData.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

UFootstepComponent::UFootstepComponent() 
{ 
    PrimaryComponentTick.bCanEverTick = false; 
}

void UFootstepComponent::PlayFootstep(FName FootBoneName)
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	FVector Start = Owner->GetMesh()->GetSocketLocation(FootBoneName);
	FVector End = Start - FVector(0.f, 0.f, 150.f);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
    Params.bReturnPhysicalMaterial = true;

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);
        UE_LOG(LogTemp, Log, TEXT("Surface Type Detected: %d"), (int32)SurfaceType);
        DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.0f, 0, 2.0f);
        // 데이터 에셋이나 TMap을 사용하여 표면별 사운드 선택
        USoundBase* SoundToPlay = Sound_DefaultStep;

        if (FootstepDataAsset)
        {
            // 현재 밟은 표면(SurfaceType)으로 데이터 검색
            const FFootstepInfo* FootStepInfo = FootstepDataAsset->SurfaceFootstepMap.Find(SurfaceType);

            // 만약 해당 표면에 대한 정보가 없다면 Default 데이터로 대체
            if (!FootStepInfo)
            {
                FootStepInfo = FootstepDataAsset->SurfaceFootstepMap.Find(SurfaceType_Default);
            }

            if (FootStepInfo)
            {
                UGameplayStatics::PlaySoundAtLocation(this, FootStepInfo->StepSound, Hit.ImpactPoint);
                UPlayerNoise::ReportNoise(GetWorld(), Owner, Hit.ImpactPoint,
                    FootStepInfo->NoiseLoudness, FootStepInfo->NoiseRange, TEXT("Footstep"));
            }
        }
    }
}