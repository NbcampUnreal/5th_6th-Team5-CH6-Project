// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Stun.h"

#include "AIController.h"
#include "MonsterAI/MonsterAI_CHS/Data/MonsterDataAsset.h"
#include "MonsterAI/MonsterAI_CHS/Entity/BaseZombie.h"

EBTNodeResult::Type UBTT_Stun::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (ABaseZombie* Zombie = Cast<ABaseZombie>(AIC->GetPawn()))
	{
		AIC->StopMovement();
		return EBTNodeResult::InProgress;
	}
	return EBTNodeResult::Failed;
}
