

#include "ButtonBase.h"

#include "Components/BoxComponent.h"


AButtonBase::AButtonBase()
{
	PrimaryActorTick.bCanEverTick = true;
	CollisionBox->SetGenerateOverlapEvents(false);
	Mesh->SetGenerateOverlapEvents(false);
	
	InteractionCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionCollisionBox"));
	InteractionCollisionBox->SetupAttachment(RootComponent);
	InteractionCollisionBox->SetGenerateOverlapEvents(true);
}

void AButtonBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AButtonBase::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (bCanInteract)
	{
		Activate();
	}
}

EInteractionType AButtonBase::GetInteractionType_Implementation() const
{
	return EInteractionType::Button;
	//return EInteractionType::Door;
}

void AButtonBase::Activate()
{
	Super::Activate();
	for (AObjectBase* Object : ActivateActors)
	{
		Object->Activate();
	}
}


