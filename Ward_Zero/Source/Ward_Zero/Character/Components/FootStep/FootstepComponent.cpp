#include "Character/Components/FootStep/FootstepComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UFootstepComponent::UFootstepComponent() { PrimaryComponentTick.bCanEverTick = false; }

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

        if (SurfaceType == SurfaceType1) 
        {
            SoundToPlay = Sound_ConcreteStep;
        }
        if (SurfaceType == SurfaceType2) 
        {
            SoundToPlay = Sound_Marble;
        }
        if (SurfaceType == SurfaceType3) 
        {
            SoundToPlay = Sound_MetalStep;
        }

        if (SoundToPlay) UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, Hit.ImpactPoint);
    }
}