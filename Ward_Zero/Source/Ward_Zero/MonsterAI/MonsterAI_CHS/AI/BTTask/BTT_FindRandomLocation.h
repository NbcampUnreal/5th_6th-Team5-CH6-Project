// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_FindRandomLocation.generated.h"

/**
 * 
 */
UCLASS()
class WARD_ZERO_API UBTT_FindRandomLocation : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTT_FindRandomLocation();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	
	UPROPERTY(EditAnywhere, Category = "Search")
	float SearchRadius = 1000.f;

	// 위치 값을 저장할 블랙보드 키 (Vector 타입)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;
};
