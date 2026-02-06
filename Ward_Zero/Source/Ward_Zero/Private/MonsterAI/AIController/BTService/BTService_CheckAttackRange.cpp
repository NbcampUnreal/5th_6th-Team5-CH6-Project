// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAI/AIController/BTService/BTService_CheckAttackRange.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterAI/AIController/WZAIKeys.h"
#include "MonsterAI/Component/StatusComponent.h"
#include "MonsterAI/Entity/BaseZombie.h"

UBTService_CheckAttackRange::UBTService_CheckAttackRange()
{
	NodeName = TEXT("Check Attack Range");
	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UBTService_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaTime)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaTime);
	
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return;
	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!BB) return;
	ABaseZombie* Zombie = Cast<ABaseZombie>(AIC->GetPawn());
	if (!Zombie) return;
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(WZAIKeys::TargetActor));
	if (!Target) return;
	float Distance = FVector::Dist(Zombie->GetActorLocation(),Target->GetActorLocation());
	float AttackRange = Zombie->GetStatusComponent()->GetAttackRange();
	BB->SetValueAsBool(WZAIKeys::IsWithinAttackRange,(Distance <= AttackRange));
	
}
