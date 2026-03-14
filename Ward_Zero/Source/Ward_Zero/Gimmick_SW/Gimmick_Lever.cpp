#include "Gimmick_SW/Gimmick_Lever.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

// Sets default values
AGimmick_Lever::AGimmick_Lever()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Lever_Frame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lever_Frame"));
	RootComponent = Lever_Frame;

	Lever_Handle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lever_Handle"));
	Lever_Handle->SetupAttachment(Lever_Frame);
	
}

// Called when the game starts or when spawned
void AGimmick_Lever::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGimmick_Lever::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGimmick_Lever::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (IInteractionBase::Execute_CanBeInteracted(this))
	{
		IInteractionBase::Execute_HandleInteraction(this, Character);

		AGimmick_Lever* Lever = Cast<AGimmick_Lever>(Lever_Handle);
		if (Lever_Handle)
		{
			FRotator Opened = Lever_Handle->GetRelativeRotation();
			Opened.Pitch = -45.0f;
			Lever_Handle->SetRelativeRotation(Opened);
		}
	}
}

void AGimmick_Lever::OnIneractionRangeEntered_Implementation()
{
}

void AGimmick_Lever::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
}

EInteractionType AGimmick_Lever::GetInteractionType_Implementation() const
{
	return EInteractionType();
}

bool AGimmick_Lever::SetBCanInteract(bool IsCanInteract)
{
	return false;
}

bool AGimmick_Lever::GetBCanInteract() const
{
	return false;
}

void AGimmick_Lever::HiddenActor()
{
}

bool AGimmick_Lever::CanBeInteracted_Implementation() const
{
	return true;
}