// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_KnockdownEnd.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "MonsterAI/MonsterAI_CHS/AI/WZAIKeys.h"
#include "MonsterAI/MonsterAI_CHS/Component/StatusComponent.h"
#include "MonsterAI/MonsterAI_CHS/Entity/BaseZombie.h"

void UAN_KnockdownEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (MeshComp && MeshComp->GetOwner())
	{
		if (ABaseZombie* Zombie = Cast<ABaseZombie>(MeshComp->GetOwner()))
		{
			Zombie->GetStatusComponent()->SetIsRecoveringCC(false);
			UE_LOG(LogTemp,Warning,TEXT("AN knockdown end: isrecoveringcc is %S"),Zombie->GetStatusComponent()->GetIsRecoveringCC() ? "true" : "false");

			if (AAIController* AIC = Cast<AAIController>(Zombie->GetController()))
			{
				AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsKnockedDown,false);
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
