// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_SW/DoorLockTrigger.h"

// Sets default values
ADoorLockTrigger::ADoorLockTrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetBoxExtent(FVector(300.0f, 300.0f, 200.0f));

	TriggerBox->SetHiddenInGame(true);
	TriggerBox->ShapeColor = FColor::Yellow;

	bHasBeenTriggered = false;
	bBossKilled = false;
}

// Called when the game starts or when spawned
void ADoorLockTrigger::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerBox->OnComponentBeginOverlap.AddDynamic(
		this, &ADoorLockTrigger::OnOverlapBegin);

}

void ADoorLockTrigger::BeginTriggered()
{
	if (OnOverlapBegin);
		//상호작용 기능 정지
}

void ADoorLockTrigger::EndTriggered()
{
	if (bBossKilled = true);
	//보스가 죽었을때 상호작용 기능 정상화
}

// Called every frame
void ADoorLockTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

