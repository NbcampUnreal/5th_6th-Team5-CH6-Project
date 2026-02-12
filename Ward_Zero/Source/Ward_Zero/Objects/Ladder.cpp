#include "Objects/Ladder.h"
#include "Components/BoxComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"

ALadder::ALadder()
{
	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	SetRootComponent(SceneComp);

	LadderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LadderMesh"));
	LadderMesh->SetupAttachment(RootComponent);

	InteractBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractBox"));
	InteractBox->SetupAttachment(RootComponent);
	InteractBox->SetBoxExtent(FVector(50.f, 50.f, 100.f));
	InteractBox->SetCollisionProfileName(TEXT("Trigger"));

	BottomStartPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BottomStartPoint"));
	BottomStartPoint->SetupAttachment(RootComponent);

	TopExitPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TopExitPoint"));
	TopExitPoint->SetupAttachment(RootComponent);

	FRotator Rot = FRotator(0, 0, -90.f);
	FVector Loc = FVector(0, 0, -90.f);
	SetActorLocationAndRotation(Loc, Rot);
	
}

void ALadder::BeginPlay()
{
	Super::BeginPlay();
}

void ALadder::OnInteract_Implementation(APrototypeCharacter* Player)
{
	if (Player)
	{
		Player->StartClimbing(this);
	}
}
