// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAI/MonsterAI_CHS/Animation/AN_MonsterAttackEnd.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTTaskNode.h"

void UAN_MonsterAttackEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                  const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (APawn* Pawn = Cast<APawn>(MeshComp->GetOwner()))
		{

			if (AAIController* AIC = Cast<AAIController>(Pawn->GetController()))
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
