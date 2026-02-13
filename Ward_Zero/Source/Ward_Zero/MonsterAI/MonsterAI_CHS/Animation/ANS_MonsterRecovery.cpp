// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAI/MonsterAI_CHS/Animation/ANS_MonsterRecovery.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterAI/MonsterAI_CHS/AI/BaseZombie_AIController.h"
#include "MonsterAI/MonsterAI_CHS/Component/StatusComponent.h"
#include "MonsterAI/MonsterAI_CHS/Entity/BaseZombie.h"
#include "MonsterAI/MonsterAI_CHS/AI/WZAIKeys.h"





void UANS_MonsterRecovery::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		ABaseZombie* Owner = Cast<ABaseZombie>(MeshComp->GetOwner());
		if (Owner && Owner->GetStatusComponent())
		{
			Owner->GetStatusComponent()->SetIsRecoveringCC(true);
		}
	}
}

void UANS_MonsterRecovery::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		ABaseZombie* Owner = Cast<ABaseZombie>(MeshComp->GetOwner());
		if (Owner)
		{
			if (Owner->GetStatusComponent())
			{
				Owner->GetStatusComponent()->SetIsRecoveringCC(false);
			}
			auto* AIC = Cast<ABaseZombie_AIController>(Owner->GetController());
			if (AIC && AIC->GetBlackboardComponent())
			{
				AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsKnockedDown,false);
				AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsStunned,false);

			}
		}
	}
}
