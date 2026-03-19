// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAI/MonsterAI_CHS/AI/BaseZombie_AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MonsterAI/MonsterAI_CHS/Component/StatusComponent.h"
#include "MonsterAI/MonsterAI_CHS/Entity/BaseZombie.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "MonsterAI/MonsterAI_CHS/AI/WZAIKeys.h"
#include "MonsterAI/MonsterAI_CHS/Data/MonsterDataAsset.h"
#include "MonsterAI/MonsterAI_CHS/Object/ZombieInteractableInterface.h"
#include "Navigation/PathFollowingComponent.h"

ABaseZombie_AIController::ABaseZombie_AIController()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	SightConfig->SightRadius = 10.f;
	SightConfig->LoseSightRadius = 10.f;
	SightConfig->PeripheralVisionAngleDegrees = 10.f;
	
	SightConfig->DetectionByAffiliation.bDetectEnemies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	
	HearingConfig->DetectionByAffiliation.bDetectEnemies = false;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = false;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
	
	
	
	HearingConfig->HearingRange = 10.f;

	AIPerceptionComp->ConfigureSense(*HearingConfig);
	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	
	
	
}

void ABaseZombie_AIController::BeginPlay()
{
	Super::BeginPlay();
	
	
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseZombie_AIController::OnTargetDetected);
	
	
}

void ABaseZombie_AIController::HandleMainStateChange(EMonsterMainState NewState)
{
	if (GetBlackboardComponent())
	{
		UE_LOG(LogTemp,Warning,TEXT("AIC_HandleMainStateChange: blackboard valid"));
		GetBlackboardComponent()->SetValueAsEnum(WZAIKeys::MainState,static_cast<uint8>(NewState));
		if (NewState == EMonsterMainState::Combat)
		{
			GetBlackboardComponent()->SetValueAsObject(WZAIKeys::TargetActor,UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		}
	
	}else
	{
		UE_LOG(LogTemp,Warning,TEXT("AIC_HandleMainStateChange: blackboard not valid"));
	}
	
}

void ABaseZombie_AIController::HandleSubStateChange(EMonsterSubState NewState)
{
	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->SetValueAsEnum(WZAIKeys::SubState,static_cast<uint8>(NewState));
	}
}

void ABaseZombie_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (ABaseZombie* Zombie = Cast<ABaseZombie>(InPawn))
	{
		if (BT_BaseZombie != nullptr)
		{
			//RunBehaviorTree(BT_BaseZombie);
		}
		StatusComp = Zombie->FindComponentByClass<UStatusComponent>();
		
		if (StatusComp)
		{
			UAIPerceptionSystem* Psys = UAIPerceptionSystem::GetCurrent(GetWorld());
			Psys->UnregisterSource(*GetOwner());
			//UpdatePerceptionConfig();
			StatusComp->OnMainStateChanged.AddDynamic(this, &ABaseZombie_AIController::HandleMainStateChange);
			StatusComp->OnSubStateChanged.AddDynamic(this, &ABaseZombie_AIController::HandleSubStateChange);
			UE_LOG(LogTemp,Warning,TEXT("call SetMainState"));
			//StatusComp->SetMainState(StatusComp->GetStartState());
			
		}
	}
}

void ABaseZombie_AIController::OnUnPossess()
{
	Super::OnUnPossess();
	StatusComp = nullptr;
}

void ABaseZombie_AIController::StopInteracting()
{
	StatusComp->SetMainState(EMonsterMainState::Combat);
	GetBlackboardComponent()->SetValueAsObject(WZAIKeys::TargetActor,UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void ABaseZombie_AIController::HandleInteractionRequest(FGameplayTag InteractingTag, const FVector& Destination,AActor* InterActor)
{
	
	UE_LOG(LogTemp,Warning,TEXT("AIC recieve interaction request"));
	if (StatusComp->GetMainState() ==  EMonsterMainState::Patrol)
	{
		/*StatusComp->SetMainState(EMonsterMainState::Idle);
		StatusComp->SetMainState(EMonsterMainState::Patrol);*/
		return;
	}
	
	ABaseZombie* Zombie = Cast<ABaseZombie>(GetPawn());
	if (!Zombie)
	{
		return;
	}

	const UMonsterDataAsset* Data = Zombie->GetMonsterData();
	if (!Data)
	{
		return;
	}
	if (!Zombie->GetMonsterData()) return;
	
	IZombieInteractableInterface* Interactable = Cast<IZombieInteractableInterface>(InterActor);
	if (Interactable)
	{
		UE_LOG(LogTemp,Warning,TEXT("AIC bind delegate to door"));
		Interactable->GetOnDestroyedDelegate().AddDynamic(this,&ABaseZombie_AIController::StopInteracting);
		
	}
	
	
	const FInteractionInfo* Info = Zombie->GetMonsterData()->InteractionInfoMap.Find(InteractingTag);
	if (Info && Info->InteractionMontage)
	{
		StatusComp->SetMainState(EMonsterMainState::Interacting);
		GetBlackboardComponent()->SetValueAsEnum(WZAIKeys::InterActingObject,static_cast<uint8>(Info->InteractableObject));
	}
}


void ABaseZombie_AIController::UpdatePerceptionConfig()
{
	if (!StatusComp || !AIPerceptionComp || !StatusComp->IsDataInit())
	{
		return;
	}
	AIPerceptionComp->ForgetAll();
	UAISenseConfig_Sight* SightConfig = AIPerceptionComp->GetSenseConfig<UAISenseConfig_Sight>();
	if (SightConfig)
	{
		SightConfig->SightRadius = StatusComp->GetBaseDetectionRange();
		SightConfig->LoseSightRadius = StatusComp->GetLoseSightRange();
		SightConfig->PeripheralVisionAngleDegrees = StatusComp->GetViewAngle() / 2;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	
		
		
		AIPerceptionComp->RequestStimuliListenerUpdate();
	}
	UAISenseConfig_Hearing* HearingConfig = AIPerceptionComp->GetSenseConfig<UAISenseConfig_Hearing>();
	if (HearingConfig)
	{
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		//HearingConfig->HearingRange = StatusComp->GetHearingRange();
	}
}

void ABaseZombie_AIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}
	
	TSubclassOf<UAISense> DetectedSense = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);
	
	if (DetectedSense == UAISense_Sight::StaticClass())
	{
		if (Actor && Actor->ActorHasTag("Player"))
		{
			if (Stimulus.WasSuccessfullySensed())
			{
				StatusComp->SetMainState(EMonsterMainState::Combat);
				BB->SetValueAsObject(WZAIKeys::TargetActor, Actor);
				BB->ClearValue(WZAIKeys::InvestigateLocation);
				BB->ClearValue(WZAIKeys::LastKnownLocation);
			
			}else
			{
				//StatusComp->SetMainState(EMonsterMainState::Investigate);
				BB->SetValueAsVector(WZAIKeys::LastKnownLocation, Stimulus.StimulusLocation);
			}
		}
	}else if (DetectedSense == UAISense_Hearing::StaticClass())
	{
		if (BB->GetValueAsObject(WZAIKeys::TargetActor) == nullptr && Stimulus.WasSuccessfullySensed())
		{
			float Loudness = Stimulus.Strength;
			float dist = FVector::Dist(GetPawn()->GetActorLocation(),Actor->GetActorLocation());
			if (dist > 2000)
			{
				return;
			}
			float RealLoudness = Loudness * FMath::Clamp((1 - dist/2000),0,1.0f);
			
			if (StatusComp->GetHearingThreshold() <= RealLoudness)
			{
				StatusComp->SetMainState(EMonsterMainState::Investigate);
				BB->SetValueAsVector(WZAIKeys::InvestigateLocation, Stimulus.StimulusLocation);
			}
		}
	}
	
}

bool ABaseZombie_AIController::Activate()
{
	if (StatusComp->GetIsDead())
	{
		return false;
	}
	if (BT_BaseZombie != nullptr)
	{
		RunBehaviorTree(BT_BaseZombie);
		if (StatusComp)
		{
			UAIPerceptionSystem* Psys = UAIPerceptionSystem::GetCurrent(GetWorld());
			Psys->RegisterSource(*GetOwner());
			StatusComp->SetMainState(StatusComp->GetStartState());
			UpdatePerceptionConfig();
			return true;
		}else
		{
			return false;
		}
	}
	return false;
}

void ABaseZombie_AIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
