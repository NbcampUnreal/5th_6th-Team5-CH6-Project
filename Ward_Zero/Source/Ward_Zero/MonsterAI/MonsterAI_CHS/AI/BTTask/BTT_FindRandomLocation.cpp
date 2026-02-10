// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FindRandomLocation.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_FindRandomLocation::UBTT_FindRandomLocation()
{
	NodeName = "Find Random Location";
}

EBTNodeResult::Type UBTT_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;
	
	APawn* Pawn = AIC->GetPawn();
	if (!Pawn) return EBTNodeResult::Failed;
	
	FVector Origin = Pawn->GetActorLocation();
	
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return EBTNodeResult::Failed;
	
	FNavLocation RandLocation;
	if (NavSys->GetRandomReachablePointInRadius(Origin, SearchRadius,RandLocation))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(TargetLocationKey.SelectedKeyName, RandLocation.Location);
		return EBTNodeResult::Succeeded;	
	}
	return EBTNodeResult::Failed;
}
