#include "ItemBase.h"
#include "Components/BoxComponent.h"
#include "UI_KWJ/Save/WardSaveGame.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetBoxExtent(FVector(10.0f, 10.0f, 10.0f));
	SetRootComponent(CollisionBox);
	//CollisionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	//RootComponent = Mesh;
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetupAttachment(CollisionBox);
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

void AItemBase::PostActorCreated()
{
	Super::PostActorCreated();

	// 액터가 에디터에 배치되거나 스폰될 때 최초 1회만 GUID 생성
	if (!ActorID.IsValid())
	{
		ActorID = FGuid::NewGuid();
		UE_LOG(LogTemp, Warning, TEXT("New Item ID Generated: %s"), *ActorID.ToString());
	}
}
//
//void AItemBase::OnConstruction(const FTransform& Transform)
//{
//	Super::OnConstruction(Transform);
//
//	if (!ActorID.IsValid())
//	{
//		ActorID = FGuid::NewGuid();
//	}
//
//	UE_LOG(LogTemp, Warning, TEXT("Item ID: %s"), *ActorID.ToString());
//}

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

