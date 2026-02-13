// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Stun.generated.h"

/**
 * 
 */
UCLASS()
class WARD_ZERO_API UBTT_Stun : public UBTTaskNode
{
	GENERATED_BODY()
	public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
