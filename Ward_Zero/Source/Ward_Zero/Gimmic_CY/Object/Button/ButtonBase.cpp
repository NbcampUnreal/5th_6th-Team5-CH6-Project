

#include "ButtonBase.h"


AButtonBase::AButtonBase()
{
	PrimaryActorTick.bCanEverTick = true;
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
	//return EInteractionType::Button;
	return EInteractionType::Door;
}

void AButtonBase::Activate()
{
	Super::Activate();
	for (AObjectBase* Object : ActivateActors)
	{
		Object->Activate();
	}
}


