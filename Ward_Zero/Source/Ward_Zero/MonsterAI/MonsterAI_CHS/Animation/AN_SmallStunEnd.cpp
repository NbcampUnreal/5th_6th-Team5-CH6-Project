// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_SmallStunEnd.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterAI/MonsterAI_CHS/AI/WZAIKeys.h"

void UAN_SmallStunEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner());
		if (OwnerPawn)
		{
			AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
			if (AIController)
			{
				if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
				{
					BB->SetValueAsBool(WZAIKeys::IsStunned, false);
				}
			}
		}
	}
}
