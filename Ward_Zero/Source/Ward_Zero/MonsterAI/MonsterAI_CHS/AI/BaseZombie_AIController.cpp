// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAI/MonsterAI_CHS/AI/BaseZombie_AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterAI/MonsterAI_CHS/Component/StatusComponent.h"
#include "MonsterAI/MonsterAI_CHS/Entity/BaseZombie.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "MonsterAI/MonsterAI_CHS/AI/WZAIKeys.h"

ABaseZombie_AIController::ABaseZombie_AIController()
{
	ConstructorHelpers::FObjectFinder<UBehaviorTree>BT(TEXT("BehaviorTree'/Game/MonsterAI_CHS/BT_BaseZombie'"));
	if (BT.Succeeded())
	{
		BT_BaseZombie = BT.Object;
	}
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	SightConfig->SightRadius = 1500.f;
	SightConfig->LoseSightRadius = 2000.f;
	SightConfig->PeripheralVisionAngleDegrees = 60.f;
	
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	
	HearingConfig->HearingRange = 2000.f;

	AIPerceptionComp->ConfigureSense(*HearingConfig);
	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ABaseZombie_AIController::BeginPlay()
{
	Super::BeginPlay();
	
	if (BT_BaseZombie != nullptr)
	{
		RunBehaviorTree(BT_BaseZombie);
	}
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseZombie_AIController::OnTargetDetected);
}

void ABaseZombie_AIController::HandleMainStateChange(EMonsterMainState NewState)
{
	GetBlackboardComponent()->SetValueAsEnum(WZAIKeys::MainState,static_cast<uint8>(NewState));
}

void ABaseZombie_AIController::HandleSubStateChange(EMonsterSubState NewState)
{
	GetBlackboardComponent()->SetValueAsEnum(WZAIKeys::SubState,static_cast<uint8>(NewState));
}

void ABaseZombie_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (ABaseZombie* Zombie = Cast<ABaseZombie>(InPawn))
	{
		StatusComp = Zombie->FindComponentByClass<UStatusComponent>();
		
		if (StatusComp)
		{
			UpdatePerceptionConfig();
			StatusComp->OnMainStateChanged.AddDynamic(this, &ABaseZombie_AIController::HandleMainStateChange);
			StatusComp->OnSubStateChanged.AddDynamic(this, &ABaseZombie_AIController::HandleSubStateChange);

		}
	}
}

void ABaseZombie_AIController::OnUnPossess()
{
	Super::OnUnPossess();
	StatusComp = nullptr;
}

void ABaseZombie_AIController::UpdatePerceptionConfig()
{
	if (!StatusComp || !AIPerceptionComp || !StatusComp->IsDataInit())
	{
		return;
	}
	UAISenseConfig_Sight* SightConfig = AIPerceptionComp->GetSenseConfig<UAISenseConfig_Sight>();
	if (SightConfig)
	{
		SightConfig->SightRadius = StatusComp->GetBaseDetectionRange();
		SightConfig->LoseSightRadius = StatusComp->GetLoseSightRange();
		SightConfig->PeripheralVisionAngleDegrees = StatusComp->GetViewAngle() / 2;
		
		AIPerceptionComp->RequestStimuliListenerUpdate();
	}
	UAISenseConfig_Hearing* HearingConfig = AIPerceptionComp->GetSenseConfig<UAISenseConfig_Hearing>();
	if (HearingConfig)
	{
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
				StatusComp->SetMainState(EMonsterMainState::Combat);
				BB->SetValueAsObject(WZAIKeys::TargetActor,Actor);
				BB->SetValueAsVector(WZAIKeys::LastKnownLocation, Stimulus.StimulusLocation);
			}else
			{
				StatusComp->SetMainState(EMonsterMainState::Investigate);
				BB->SetValueAsVector(WZAIKeys::InvestigateLocation, Stimulus.StimulusLocation);
			}
			
		}
	}
	
}

void ABaseZombie_AIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
