// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAI/Animation/AN_MonsterAttackEnd.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "MonsterAI/AIController/WZAIKeys.h"

void UAN_MonsterAttackEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                  const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	UE_LOG(LogTemp,Warning,TEXT("Notify Start"));

	if (MeshComp && MeshComp->GetOwner())
	{
		if (APawn* Pawn = Cast<APawn>(MeshComp->GetOwner()))
		{
			UE_LOG(LogTemp,Warning,TEXT("Pawn clear"));

			if (AAIController* AIC = Cast<AAIController>(Pawn->GetController()))
			{
				UE_LOG(LogTemp,Warning,TEXT("AIC clear"));

				if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
				{
					UE_LOG(LogTemp,Warning,TEXT("BB clear"));
					BB->SetValueAsBool(WZAIKeys::IsAttacking,false);
				}
				if (UBehaviorTreeComponent* BT = Cast<UBehaviorTreeComponent>(AIC->GetBrainComponent()))
				{
					UE_LOG(LogTemp,Warning,TEXT("BT clear"));
					if (const UBTNode* ActiveNode = BT->GetActiveNode())
					{
						UE_LOG(LogTemp,Warning,TEXT("Task Stop"));
						BT->OnTaskFinished(Cast<UBTTaskNode>(ActiveNode),EBTNodeResult::Succeeded);
					}
				}
			}
		}
	}
}
