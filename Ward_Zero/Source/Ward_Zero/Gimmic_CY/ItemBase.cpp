#include "ItemBase.h"
#include "UI_KWJ/Save/WardSaveGame.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();	
}

void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemBase::OnIneractionRangeEntered_Implementation()
{
}

void AItemBase::OnIneractionRangeExited_Implementation()
{
}

void AItemBase::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (IInteractionBase::Execute_CanBeInteracted(this))
	{
		IInteractionBase::Execute_HandleInteraction(this, Character);
	}
}

void AItemBase::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	bCollected = true;

	HiddenActor();
	///GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "HiidenActor");
}

EInteractionType AItemBase::GetInteractionType_Implementation() const
{
	return EInteractionType();
}

bool AItemBase::SetBCanInteract(bool IsCanInteract)
{
	bCanInteract = IsCanInteract;
	return bCanInteract;
}

bool AItemBase::GetBCanInteract() const
{
	return bCanInteract;
}

void AItemBase::HiddenActor()
{
	Mesh->SetVisibility(false);
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "SetVisibility(false)");
}

void AItemBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!ActorID.IsValid())
	{
		ActorID = FGuid::NewGuid();
	}

	UE_LOG(LogTemp, Warning, TEXT("Item ID: %s"), *ActorID.ToString());
}

FGuid AItemBase::GetActorID() const
{
	return ActorID;
}

void AItemBase::SaveActorState(UWardSaveGame* SaveData)
{
	//if (bCollected)
	//{
	//	SaveData->CollectedItems.Add(ActorID);
	//}
}

void AItemBase::LoadActorState(UWardSaveGame* SaveData)
{
	//if (SaveData->CollectedItems.Contains(ActorID))
	//{
	//	Destroy();
	//}
}

