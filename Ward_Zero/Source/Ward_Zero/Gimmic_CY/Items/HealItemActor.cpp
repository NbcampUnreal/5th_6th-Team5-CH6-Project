#include "Gimmic_CY/Items/HealItemActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Status/PlayerStatusComponent.h"

AHealItemActor::AHealItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	//// 충돌 박스
	////CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	//SetRootComponent(CollisionBox);
	//CollisionBox->SetBoxExtent(FVector(20.0f, 20.0f, 20.0f));
	//CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	

	// 약병 뚜껑 메쉬 
	CapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CapMesh"));
	CapMesh->SetupAttachment(Mesh); 
	CapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	
}

void AHealItemActor::BeginPlay()
{
	Super::BeginPlay();
	
}


void AHealItemActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!bCanInteract)
		return;
	Super::HandleInteraction_Implementation(Character);
	
	if (Character && Character->StatusComp)
	{
		// 회복약 개수 증가 시도
		if (Character->StatusComp->AddHealingItem(1))
		{
			UE_LOG(LogTemp, Warning, TEXT("Heal Item Acquired!"));
		}
	}
	
}

EInteractionType AHealItemActor::GetInteractionType_Implementation() const
{
	// Enum에 Heal이 없다면 임시로 Ammo 사용, 있다면 Heal로 변경
	return EInteractionType::Heal;
}

void AHealItemActor::HiddenActor()
{
	Super::HiddenActor();
	CapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapMesh->SetVisibility(false);
}

