// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAI/MonsterAI_CHS/AI/BTTask/BTT_MeleeAttack.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterAI/MonsterAI_CHS/AI/WZAIKeys.h"
#include "MonsterAI/MonsterAI_CHS/Component/CombatComponent.h"
#include "MonsterAI/MonsterAI_CHS/Data/MonsterDataAsset.h"
#include "MonsterAI/MonsterAI_CHS/Entity/BaseZombie.h"

EBTNodeResult::Type UBTT_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	UE_LOG(LogTemp,Warning,TEXT("Melee Attack start"));
	if (ABaseZombie* Zombie = Cast<ABaseZombie>(AIC->GetPawn()))
	{
		AIC->StopMovement();
		if (Zombie->MonsterData->AttackMontage)
		{
			UE_LOG(LogTemp,Warning,TEXT("Set IsAttacking blackboard key true"))
			AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsAttacking,true);
		}
		Zombie->GetCombatComponent()->Attack();
		return EBTNodeResult::InProgress;
	}
	return EBTNodeResult::Failed;
}
