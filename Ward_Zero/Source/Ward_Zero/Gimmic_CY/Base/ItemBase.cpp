#include "ItemBase.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "UI_KWJ/Save/WardSaveGame.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetBoxExtent(FVector(10.0f, 10.0f, 10.0f));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetupAttachment(CollisionBox);
	

	PickUpPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PickUpPoint"));
	PickUpPoint->SetupAttachment(Mesh); 
	PickUpPoint->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
	
	
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();	
	//to do: bIsActivate = SaveManager(ActorID)
	bool bIsActivate = true;
	if (!bIsActivate)
	{
		bCanInteract = false;
		HiddenActor();
	}
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
	if (!bCanInteract)
		return;

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
	
	//todo: SaveManager->SetActorInteractable(ActorId,bCanInteract)
	return bCanInteract;
}

bool AItemBase::GetBCanInteract() const
{
	return bCanInteract;
}

void AItemBase::HiddenActor()
{
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetVisibility(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "SetVisibility(false)");
}

void AItemBase::PostActorCreated()
{
	Super::PostActorCreated();

	// ���Ͱ� �����Ϳ� ��ġ�ǰų� ������ �� ���� 1ȸ�� GUID ����
	if (!ActorID.IsValid())
	{
		ActorID = FGuid::NewGuid();
		UE_LOG(LogTemp, Warning, TEXT("New Item ID Generated: %s"), *ActorID.ToString());
	}
}

//FGuid AItemBase::GetActorID() const
//{
//	return ActorID;
//}
//
//void AItemBase::SaveActorState(UWardSaveGame* SaveData)
//{
//	//if (bCollected)
//	//{
//	//	SaveData->CollectedItems.Add(ActorID);
//	//}
//}
//
//void AItemBase::LoadActorState(UWardSaveGame* SaveData)
//{
//	//if (SaveData->CollectedItems.Contains(ActorID))
//	//{
//	//	Destroy();
//	//}
//}

FVector AItemBase::GetInteractionTargetLocation_Implementation() const
{
	return GetActorLocation();
}

