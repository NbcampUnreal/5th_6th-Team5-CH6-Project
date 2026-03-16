#include "Gimmic_CY/BasicDoorActor.h"

ABasicDoorActor::ABasicDoorActor()
{
	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	//Door->SetupAttachment(InteractionBox);
	Door->SetCollisionResponseToChannels(ECR_Block);
}

EInteractionType ABasicDoorActor::GetInteractionType_Implementation() const
{
	return EInteractionType::Door;
}
