// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_SystemDoor.h"

// Sets default values
AGimmick_SystemDoor::AGimmick_SystemDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AGimmick_SystemDoor::OpenDoor()
{
}

// Called when the game starts or when spawned
void AGimmick_SystemDoor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGimmick_SystemDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
