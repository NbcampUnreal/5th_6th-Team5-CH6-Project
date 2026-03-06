#include "Gimmick_SW/DoorLockTrigger.h"

// Sets default values
ADoorLockTrigger::ADoorLockTrigger()
{
	PrimaryActorTick.bCanEverTick = true;

	/*TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));

	TriggerBox->SetHiddenInGame(true);
	TriggerBox->ShapeColor = FColor::Yellow;

	bHasBeenTriggered = false;
	bBossKilled = false;*/
}

void ADoorLockTrigger::BeginPlay()
{
	Super::BeginPlay();
	
	//TriggerBox->OnComponentBeginOverlap.AddDynamic(
	//	this, &ADoorLockTrigger::OnOverlapBegin);

}

void ADoorLockTrigger::BeginTriggered()
{
	//if (OnOverlapBegin) 
	

	
		//상호작용 기능 정지
}

void ADoorLockTrigger::EndTriggered()
{
	//if (bBossKilled = true);
	

	
	//보스가 죽었을때 상호작용 기능 정상화
}

// Called every frame
void ADoorLockTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

