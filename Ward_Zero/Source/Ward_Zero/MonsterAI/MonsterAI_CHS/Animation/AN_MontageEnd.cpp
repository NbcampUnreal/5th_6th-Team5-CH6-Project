// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_MontageEnd.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "MonsterAI/MonsterAI_CHS/Component/CombatComponent.h"
#include "MonsterAI/MonsterAI_CHS/Component/StatusComponent.h"
#include "MonsterAI/MonsterAI_CHS/Entity/BaseZombie.h"

void UAN_MontageEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                            const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ABaseZombie* Zombie = Cast<ABaseZombie>(MeshComp->GetOwner()))
		{
			Zombie->GetCombatComponent()->SetIsAttacking(false);

			if (AAIController* AIC = Cast<AAIController>(Zombie->GetController()))
			{
				if (UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(AIC->GetBrainComponent()))
				{
					if (const UBTNode* ActiveNode = BT->GetActiveNode())
					{
						BT->OnTaskFinished(Cast<UBTTaskNode>(ActiveNode),EBTNodeResult::Succeeded);
					}
				}
			}
		}
	}
}
