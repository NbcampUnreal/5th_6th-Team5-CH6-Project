// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick_SystemDoor.h"

// Sets default values
AGimmick_SystemDoor::AGimmick_SystemDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	OpenAngle = 90.f;
	RotationSpeed = 45.f;
	WaitTimeBeforeClose = 2.f;

	bHaveCardKey = false;
	bIsSystemDoorOpening = false;
	bIsRotating = false;
	bIsClosing = false;
	CurrentYaw = 0.f;
}

// Called when the game starts or when spawned
void AGimmick_SystemDoor::BeginPlay()
{
	Super::BeginPlay();
	StartRotation = GetActorRotation();
	
}

void AGimmick_SystemDoor::OpenSystemDoor()
{
	if (bHaveCardKey) return; // 진행중일때 중복실행 방지

	bIsSystemDoorOpening = true;
	bIsRotating = true;
	CurrentYaw = 0.f;

	TargetRotation = StartRotation + FRotator(0.f, OpenAngle, 0.f);
}

void AGimmick_SystemDoor::CloseSystemDoor()
{
	if (bIsClosing) return;

	bIsClosing = true;
	bIsRotating = true;
	CurrentYaw = OpenAngle;

	TargetRotation = StartRotation;
}

// Called every frame
void AGimmick_SystemDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
