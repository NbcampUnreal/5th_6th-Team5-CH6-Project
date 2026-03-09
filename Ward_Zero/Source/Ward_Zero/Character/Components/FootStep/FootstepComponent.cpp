#include "Character/Components/FootStep/FootstepComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UFootstepComponent::UFootstepComponent() { PrimaryComponentTick.bCanEverTick = false; }

void UFootstepComponent::PlayFootstep(FName FootBoneName)
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	FVector Start = Owner->GetMesh()->GetSocketLocation(FootBoneName);
	FVector End = Start - FVector(0.f, 0.f, 50.f);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		USoundBase* Sound = Sound_DefaultStep;
		if (Hit.GetActor() && Hit.GetActor()->ActorHasTag(TEXT("Metal"))) Sound = Sound_MetalStep;

		if (Sound) UGameplayStatics::PlaySoundAtLocation(this, Sound, Hit.ImpactPoint);
	}
}