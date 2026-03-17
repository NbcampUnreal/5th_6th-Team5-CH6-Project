#include "Gimmic_CY/Items/AmmoBoxActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "Weapon/Weapon.h"
#include "Components/WidgetComponent.h"

AAmmoBoxActor::AAmmoBoxActor()
{
	PrimaryActorTick.bCanEverTick = false;

	//CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	//CollisionBox->SetupAttachment(RootComponent);
	//CollisionBox->SetBoxExtent(FVector(10.0f, 10.0f, 10.0f));
	////SetRootComponent(CollisionBox);
	////CollisionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	//CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	//MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	//MeshComp->SetupAttachment(RootComponent);
	//MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//MarkerPillar = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerPillar"));
	//MarkerPillar->SetupAttachment(RootComponent);
	//MarkerPillar->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	//MarkerPillar->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.5f));
	//MarkerPillar->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	//InteractWidget->SetupAttachment(RootComponent);
	//InteractWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	//InteractWidget->SetWidgetSpace(EWidgetSpace::Screen);
	//InteractWidget->SetDrawSize(FVector2D(50.0f, 50.0f));

	PickUpPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PickUpPoint"));
	PickUpPoint->SetupAttachment(Mesh);
	PickUpPoint->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
}

void AAmmoBoxActor::BeginPlay()
{
	Super::BeginPlay();

	if (InteractWidget)
	{
		InteractWidget->SetVisibility(false);
	}

	if (CollisionBox)
	{
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AAmmoBoxActor::OnOverlapBegin);
		CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AAmmoBoxActor::OnOverlapEnd);
	}
}

void AAmmoBoxActor::HandleInteraction_Implementation(APrototypeCharacter* Character)
{
	if (!Character) return;

	Super::HandleInteraction_Implementation(Character);

	UPlayerCombatComponent* CombatComp = Character->FindComponentByClass<UPlayerCombatComponent>();
	if (CombatComp)
	{
		AWeapon* TargetWeapon = nullptr;

		if (TargetWeaponIndex == 1) TargetWeapon = CombatComp->PistolWeapon;
		else if (TargetWeaponIndex == 2) TargetWeapon = CombatComp->SMGWeapon;

		if (TargetWeapon)
		{
			TargetWeapon->AddAmmo(AmmoAmount);

			UE_LOG(LogTemp, Warning, TEXT("Looted %d Ammo for Weapon %d"), AmmoAmount, TargetWeaponIndex);
			
			//Destroy();
		}
	}
}

EInteractionType AAmmoBoxActor::GetInteractionType_Implementation() const
{
	return EInteractionType::Ammo;
}
//
//void AAmmoBoxActor::HiddenActor()
//{
//}

void AAmmoBoxActor::OnIneractionRangeEntered_Implementation()
{
	//UE_LOG(LogTemp, Warning, TEXT("Entered Ammo Range"));
	//if (InteractWidget)
	//{
	//	InteractWidget->SetVisibility(true);
	//}
}

void AAmmoBoxActor::OnIneractionRangeExited_Implementation()
{
	//UE_LOG(LogTemp, Warning, TEXT("Exited Ammo Range"));
	//if (InteractWidget)
	//{
	//	InteractWidget->SetVisibility(false);
	//}
}

void AAmmoBoxActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bCanInteract)
		return;

	//if (OtherActor && OtherActor->IsA(APrototypeCharacter::StaticClass()))
	//{
	//	// 맞다면 동그라미를 켜는 함수를 실행!
	//	//OnIneractionRangeEntered();
	//	if (Cast<APrototypeCharacter>(OtherActor))
	//	{
	//		IInteractionBase::Execute_OnIneractionRangeEntered(this);
	//	}
	//}
}

void AAmmoBoxActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!bCanInteract)
		return;

	//if (OtherActor && OtherActor->IsA(APrototypeCharacter::StaticClass()))
	//{
	//	// 맞다면 동그라미를 끄는 함수를 실행!
	//	//OnIneractionRangeExited();
	//	IInteractionBase::Execute_OnIneractionRangeExited(this);
	//}
}

FVector AAmmoBoxActor::GetInteractionTargetLocation_Implementation() const
{
	if (PickUpPoint)
	{
		return PickUpPoint->GetComponentLocation();
	}
	return GetActorLocation();
}
