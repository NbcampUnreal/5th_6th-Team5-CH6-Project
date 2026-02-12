// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_SW/SystemDoor.h"

// Sets default values
ASystemDoor::ASystemDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASystemDoor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASystemDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

