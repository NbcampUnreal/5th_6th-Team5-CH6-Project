#include "Gimmic_CY/Test/InteractTrigger.h"
#include "Components/BoxComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Gimmic_CY/Test/PlayerHUD.h"
#include "Components/WidgetComponent.h"


AInteractTrigger::AInteractTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));

	static ConstructorHelpers::FClassFinder<UPlayerHUD> WidgetBP(
		TEXT("/Game/Gimmick/Gimmick_CY/Gimmick_BluePrints/WB_Locker.WB_Locker_C"));

	if (WidgetBP.Succeeded())
	{
		PlayerHUDClass = WidgetBP.Class;
	}

}

// Called when the game starts or when spawned
void AInteractTrigger::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AInteractTrigger::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AInteractTrigger::OnOverlapEnd);
}

// Called every frame
void AInteractTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInteractTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	APrototypeCharacter* Character = Cast<APrototypeCharacter>(OtherActor);
	if (!Character) return;

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC) return;

	if (!PlayerHUD&& PlayerHUDClass)
	{
		PlayerHUD = CreateWidget<UPlayerHUD>(PC, PlayerHUDClass);
		if (PlayerHUD)
		{
			PlayerHUD->AddToViewport(100);
		}
	}

	if (PlayerHUD)
	{
		PlayerHUD->SetVisibility(ESlateVisibility::Visible);

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(PlayerHUD->TakeWidget());

		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);

		PlayerHUD->SetPasscode(DoorPasscode, Door, PC);
		PlayerHUD->ShowPasscode(true);
	}
}

void AInteractTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	APrototypeCharacter* Character = Cast<APrototypeCharacter>(OtherActor);
	if (!Character) return;

	if (PlayerHUD)
	{
		PlayerHUD->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AInteractTrigger::SetDoorPasscode(int32 Passcode)
{
	if (IsValid(PlayerHUD))
	{
		PlayerHUD->SetPasscode(Passcode, Door, PCTestob);
	}
}
