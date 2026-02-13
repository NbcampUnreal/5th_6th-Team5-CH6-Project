// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"

#include "NavigationSystem.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Null.h"


ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;
	DoorMesh->SetCollisionProfileName(TEXT("BlockAll"));
	
	NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));
	NavModifier->SetAreaClass(UNavArea_Null::StaticClass());
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	UpdateNavigationState();
}

float ADoor::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator,
	AActor* DamageCauser)
{
	if (bIsBroken) return 0.0f;
	
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0, MaxHealth);
	
	UE_LOG(LogTemp, Warning, TEXT("Door HP: %f / %f"),CurrentHealth,MaxHealth);
	
	if (CurrentHealth <= 0.0f)
	{
		HandleDestruction();
	}
	return ActualDamage;
}

float ADoor::GetEntityHealth() const
{
	return CurrentHealth;
}

void ADoor::OpenDoor()
{
	if (bIsBroken || bIsOpen) return;
	bIsOpen = true;
	UpdateNavigationState();
}

void ADoor::CloseDoor()
{
	if (bIsBroken || !bIsOpen) return;
	bIsOpen = false;
	UpdateNavigationState();
}

void ADoor::ToggleDoor()
{
}

void ADoor::UpdateNavigationState()
{
	bool bIsPassable = bIsBroken || bIsOpen;
	
	if (NavModifier)
	{
		UClass* NewAreaClass = bIsPassable ? UNavArea_Default::StaticClass() : UNavArea_Null::StaticClass();
		NavModifier->SetAreaClass(NewAreaClass);
	}
	
	if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		NavSys->UpdateActorInNavOctree(*this);
		if (LinkedProxy)
		{
			LinkedProxy->SetSmartLinkEnabled(!bIsPassable);
			NavSys->UpdateActorInNavOctree(*LinkedProxy);
		}
	}
	
}

void ADoor::HandleDestruction()
{
	if (bIsBroken) return;
	
	bIsBroken = true;
	
	DoorMesh->SetVisibility(false);
	DoorMesh->SetCollisionProfileName(TEXT("NoCollision"));
	
	UpdateNavigationState();
	
	if (OnDoorDestroyed.IsBound())
	{
		OnDoorDestroyed.Broadcast();
	}
}

