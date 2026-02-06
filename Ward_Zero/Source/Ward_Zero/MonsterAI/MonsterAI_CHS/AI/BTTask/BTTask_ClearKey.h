// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ClearKey.generated.h"

/**
 * 
 */
UCLASS()
class WARD_ZERO_API UBTTask_ClearKey : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_ClearKey();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BlackboardKey;
};
