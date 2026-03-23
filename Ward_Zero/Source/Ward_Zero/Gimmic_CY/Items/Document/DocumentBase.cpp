// Fill out your copyright notice in the Description page of Project Settings.


#include "DocumentBase.h"


// Sets default values
ADocumentBase::ADocumentBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ADocumentBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADocumentBase::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	Super::HandleInteraction_Implementation(Character);
}

EInteractionType ADocumentBase::GetInteractionType_Implementation() const
{
	return EInteractionType::Document;
}



