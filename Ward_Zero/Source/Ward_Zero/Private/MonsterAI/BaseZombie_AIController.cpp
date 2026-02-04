// Fill out your copyright notice in the Description page of Project Settings.


#include "Public/MonsterAI/BaseZombie_AIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

ABaseZombie_AIController::ABaseZombie_AIController()
{
	ConstructorHelpers::FObjectFinder<UBehaviorTree>BT(TEXT("BehaviorTree'/Game/MonsterAI_CHS/BT_BaseZombie'"));
	if (BT.Succeeded())
	{
		BT_BaseZombie = BT.Object;
	}
}

void ABaseZombie_AIController::BeginPlay()
{
	Super::BeginPlay();
	
	if (BT_BaseZombie != nullptr)
	{
		RunBehaviorTree(BT_BaseZombie);
		APawn* playerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(),0);
		if (playerPawn != nullptr)
		{
			GetBlackboardComponent()->SetValueAsVector(TEXT("PlayerLocation"),playerPawn->GetActorLocation());

		}
		GetBlackboardComponent()->SetValueAsVector(TEXT("DefaultLocation"),GetPawn()->GetActorLocation());
	}
}

void ABaseZombie_AIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
