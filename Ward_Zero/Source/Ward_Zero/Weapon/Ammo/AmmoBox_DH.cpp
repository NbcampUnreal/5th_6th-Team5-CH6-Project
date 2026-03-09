#include "AmmoBox_DH.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "Weapon/Weapon.h"
#include "Components/WidgetComponent.h"

AAmmoBox_DH::AAmmoBox_DH()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
	CollisionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MarkerPillar = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerPillar"));
	MarkerPillar->SetupAttachment(RootComponent);
	MarkerPillar->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	MarkerPillar->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.5f));
	MarkerPillar->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	InteractWidget->SetupAttachment(RootComponent);
	InteractWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	InteractWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InteractWidget->SetDrawSize(FVector2D(50.0f, 50.0f));
}

void AAmmoBox_DH::BeginPlay()
{
	Super::BeginPlay();

	if (InteractWidget)
	{
		InteractWidget->SetVisibility(false);
	}

	if (CollisionBox)
	{
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AAmmoBox_DH::OnOverlapBegin);
		CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AAmmoBox_DH::OnOverlapEnd);
	}
}

void AAmmoBox_DH::OnIneracted_Implementation(APrototypeCharacter* Character)
{
	if (!Character) return;

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

			Destroy();
		}
	}
}

void AAmmoBox_DH::OnIneractionRangeEntered_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Entered Ammo Range"));
	if (InteractWidget)
	{
		InteractWidget->SetVisibility(true);
	}
}

void AAmmoBox_DH::OnIneractionRangeExited_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Exited Ammo Range"));
	if (InteractWidget)
	{
		InteractWidget->SetVisibility(false);
	}
}

void AAmmoBox_DH::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(APrototypeCharacter::StaticClass()))
	{
		// 맞다면 동그라미를 켜는 함수를 실행!
		//OnIneractionRangeEntered();
		if (Cast<APrototypeCharacter>(OtherActor))
		{
			IInteractionBase::Execute_OnIneractionRangeEntered(this);
		}
	}
}

void AAmmoBox_DH::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->IsA(APrototypeCharacter::StaticClass()))
	{
		// 맞다면 동그라미를 끄는 함수를 실행!
		//OnIneractionRangeExited();
		IInteractionBase::Execute_OnIneractionRangeExited(this);
	}
}