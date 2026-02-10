#include "Objects/BaseObject.h"

ABaseObject::ABaseObject()
{
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseObject::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseObject::OnInteract_Implementation(APrototypeCharacter* Player)
{

}

