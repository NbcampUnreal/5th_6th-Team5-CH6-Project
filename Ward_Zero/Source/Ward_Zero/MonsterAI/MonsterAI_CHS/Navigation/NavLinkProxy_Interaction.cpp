// Fill out your copyright notice in the Description page of Project Settings.


#include "NavLinkProxy_Interaction.h"

#include "MonsterAI/MonsterAI_CHS/AI/BaseZombie_AIController.h"


ANavLinkProxy_Interaction::ANavLinkProxy_Interaction()
{
}

void ANavLinkProxy_Interaction::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning,TEXT("Link Bind"));
	OnSmartLinkReached.AddDynamic(this, &ANavLinkProxy_Interaction::HandleSmartLinkReached);
}

void ANavLinkProxy_Interaction::HandleSmartLinkReached(AActor* Agent, const FVector& Destination)
{
	ABaseZombie_AIController* AIC = Cast<ABaseZombie_AIController>(Cast<APawn>(Agent)->GetController());
	UE_LOG(LogTemp, Warning,TEXT("Link Reached"));

	if (AIC)
	{
		UE_LOG(LogTemp, Warning,TEXT("Link call AIC"));
		AIC->StopMovement();
		AIC->HandleInteractionRequest(InteractionTag, Destination,InteractedObject);
	}
}
