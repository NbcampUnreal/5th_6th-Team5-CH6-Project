// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAI/MonsterAI_CHS/Entity/BaseZombie.h"

#include "BrainComponent.h"
//#include "UnrealEdGlobals.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterAI/MonsterAI_CHS/Data/MonsterDataAsset.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
//#include "Editor/UnrealEdEngine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MonsterAI/MonsterAI_CHS/AI/BaseZombie_AIController.h"
#include "MonsterAI/MonsterAI_CHS/AI/WZAIKeys.h"
#include "MonsterAI/MonsterAI_CHS/Component/CombatComponent.h"
#include "MonsterAI/MonsterAI_CHS/Component/StatusComponent.h"


// Sets default values
ABaseZombie::ABaseZombie()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StatusComponent  = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	AudioLoopComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioLoopComponent"));
	AudioLoopComponent->SetupAttachment(GetRootComponent());
	AudioLoopComponent->SetAutoActivate(false);
	
	// visualize capsule comp
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetHiddenInGame(false);
	}
}

void ABaseZombie::OnDeath()
{
	if (StatusComponent)
	{
		StatusComponent->SetIsDead(true);
		StatusComponent->SetMainState(EMonsterMainState::Dead);
	}
	if (auto* AIC = Cast<ABaseZombie_AIController>(GetController()))
	{
		AIC->StopMovement();
		if (AIC->GetBrainComponent())
		{
			AIC->GetBrainComponent()->StopLogic("Death");
		}
	}
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	if (GetMesh())
	{
		
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetSimulatePhysics(true);
	}
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetComponentTickEnabled(false);
	}
	//SetLifeSpan(5.0f);
}

// Called when the game starts or when spawned
void ABaseZombie::BeginPlay()
{
	Super::BeginPlay();
	if (StatusComponent && MonsterData)
	{
		StatusComponent->InitData(MonsterData);
		StatusComponent->OnMainStateChanged.AddDynamic(this, &ABaseZombie::HandleStateChange);
		//StatusComponent->SetMainState(StatusComponent->GetStartState());
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("Zombie [%s] has no StatusComponent or MonsterData!"), *GetName());
		return; 
	}
	
	if (ABaseZombie_AIController* AIC = GetController<ABaseZombie_AIController>())
	{
		AIC->UpdatePerceptionConfig();
	}
	bUseControllerRotationYaw = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = true;
		MoveComp->RotationRate = FRotator(0.f, MonsterData->StateConfigMap[EMonsterMainState::Idle].YawRotateSpeed, 0.f);
	}
	/*bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;*/
	
	
}

float ABaseZombie::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (CombatComponent)
	{
		
		CombatComponent->HandleAllDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
	}

	return ActualDamage;
}

// Called every frame
void ABaseZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
#if WITH_EDITOR
	if (MonsterData && StatusComponent)
	{
		//FlushPersistentDebugLines(GetWorld());

		FVector Center = GetActorLocation();
		Center.Z += MonsterData->EyeHeight;

		float View_Range = MonsterData->BaseDetectionRange;
		float View_Angle = MonsterData->ViewAngle;

		FVector Forward = GetActorForwardVector();
		FVector Right = GetActorRightVector();
		FVector Up = GetActorUpVector();

		// [수정됨] 원을 바닥에 눕히기 위해 Matrix의 첫 번째 인자(X축/Normal)를 Up 벡터로 변경
		// 순서: (Normal, YAxis, ZAxis, Origin)
		// Up 벡터가 Normal이 되면 원은 수평면(Forward-Right 평면)에 그려집니다.
		DrawDebugCircle(
			GetWorld(), 
			FMatrix(Up, Right, Forward, Center), 
			View_Range, 
			32, 
			FColor::Green, 
			false, 
			1.f, 
			0, 
			2.0f, 
			false 
		);
		

		// 시야각(부채꼴) 선 그리기 (이 부분은 기존과 동일)
		FVector LeftDir = Forward.RotateAngleAxis(-View_Angle * 0.5f, FVector::UpVector);
		FVector RightDir = Forward.RotateAngleAxis(View_Angle * 0.5f, FVector::UpVector);

		DrawDebugLine(GetWorld(), Center, Center + LeftDir * View_Range, FColor::Green, false, 0.1f, 0, 2.0f);
		DrawDebugLine(GetWorld(), Center, Center + RightDir * View_Range, FColor::Green, false, 0.1f, 0, 2.0f);
	}
#endif
	

}

void ABaseZombie::RefreshMonster()
{
	OnConstruction(GetActorTransform());
	
	UE_LOG(LogTemp, Log, TEXT("Monster Refreshed!"));
}

float ABaseZombie::GetBaseSpeed()
{
	if (StatusComponent)
	{
		return StatusComponent->GetBaseSpeed();
	}else
	{
		return 0.f;
	}
}

void ABaseZombie::SetBaseSpeed(float NewSpeed)
{
	if (StatusComponent)
	{
		StatusComponent->SetBaseSpeed(NewSpeed);
	}
}

float ABaseZombie::GetChaseSpeed()
{
	if (StatusComponent)
	{
		return StatusComponent->GetChaseSpeed();
	}else
	{
		return 0.f;
	}
}

void ABaseZombie::SetChaseSpeed(float NewSpeed)
{
	if (StatusComponent)
	{
		StatusComponent->SetChaseSpeed(NewSpeed);
	}
}

void ABaseZombie::PlayAnimM(UAnimMontage* MontageToPlay)
{
	
	PlayAnimMontage(MontageToPlay);
}

void ABaseZombie::BangDoor(AActor* TargetDoor)
{
}

void ABaseZombie::StartRagdollKnockdown(EHitDirection HitDir)
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	USkeletalMeshComponent* ZombieMesh = GetMesh();
	ZombieMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	ZombieMesh->SetAllBodiesSimulatePhysics(true);
	ZombieMesh->SetSimulatePhysics(true);
	
	if (auto* AIC = Cast<ABaseZombie_AIController>(GetController()))
	{
		AIC->StopMovement();
		AIC->GetBlackboardComponent()->SetValueAsBool(WZAIKeys::IsKnockedDown, true);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RagdollTimerHandle,
			this,
			&ABaseZombie::CheckRagdollVelocity,
			0.15f,
			true
			);
	}
}

void ABaseZombie::CheckRagdollVelocity()
{
	USkeletalMeshComponent* ZombieMesh = GetMesh();
	if (ZombieMesh->GetPhysicsLinearVelocity().Size() < 10.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RagdollTimerHandle);
		}
		RecoverFromRagdoll();
	}
	
}

void ABaseZombie::RecoverFromRagdoll()
{
	USkeletalMeshComponent* ZombieMesh = GetMesh();
	FVector PelvisLocation = ZombieMesh->GetSocketLocation(TEXT("Pelvis"));
	FVector NewLocation = PelvisLocation;
	NewLocation.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	SetActorLocation(NewLocation);
	
	FRotator PelvisRot = ZombieMesh->GetSocketRotation(TEXT("Pelvis"));
	FVector PelvisUp = FRotationMatrix(PelvisRot).GetScaledAxis(EAxis::Z);
	bool bIsFaceUp = (PelvisUp.Z > 0.0f);
	
	ZombieMesh->SetSimulatePhysics(false);
	ZombieMesh->SetCollisionProfileName(TEXT("Custom"));
	ZombieMesh->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
	ZombieMesh->AttachToComponent(GetCapsuleComponent(),FAttachmentTransformRules::SnapToTargetIncludingScale);
	ZombieMesh->SetRelativeLocationAndRotation(FVector(0,0,-90),FRotator(0,-90,0));
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	UAnimMontage* MontageToPlay = bIsFaceUp ? MonsterData->GetUpMontages.FromFaceUp : MonsterData->GetUpMontages.FromFaceDown;
	if (MontageToPlay)
	{
		PlayAnimM(MontageToPlay);
	}
}

const UMonsterDataAsset* ABaseZombie::GetMonsterData() const
{
	return MonsterData;
}

#if WITH_EDITOR
void ABaseZombie::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(ABaseZombie, MonsterData))
	{
		
		OnConstruction(GetActorTransform());
		
		/*if (GUnrealEd)
		{
			GUnrealEd->RedrawLevelEditingViewports();
		}*/
	}
}
#endif

void ABaseZombie::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (MonsterData)
	{
		
		if (MonsterData->MonsterMesh)
		{
			GetMesh()->SetSkeletalMesh(MonsterData->MonsterMesh);
		}
        
		
		if (MonsterData->AnimBP)
		{
			GetMesh()->SetAnimInstanceClass(MonsterData->AnimBP);
		}

		
		GetMesh()->SetRelativeScale3D(MonsterData->MeshScale);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		GetMesh()->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
		GetMesh()->bReturnMaterialOnMove = true;
		
		if (GetCharacterMovement())
		{
			if (MonsterData->StateConfigMap.Find(EMonsterMainState::Idle) != nullptr)
			{
				GetCharacterMovement()->MaxWalkSpeed = MonsterData->StateConfigMap[EMonsterMainState::Idle].MovementSpeed;
			}
		}
	}
	
#if WITH_EDITOR
	if (MonsterData && StatusComponent)
	{
		FlushPersistentDebugLines(GetWorld());

		FVector Center = GetActorLocation();
		Center.Z += MonsterData->EyeHeight;

		float View_Range = MonsterData->BaseDetectionRange;
		float View_Angle = MonsterData->ViewAngle;

		FVector Forward = GetActorForwardVector();
		FVector Right = GetActorRightVector();
		FVector Up = GetActorUpVector();

		// [수정됨] 원을 바닥에 눕히기 위해 Matrix의 첫 번째 인자(X축/Normal)를 Up 벡터로 변경
		// 순서: (Normal, YAxis, ZAxis, Origin)
		// Up 벡터가 Normal이 되면 원은 수평면(Forward-Right 평면)에 그려집니다.
		DrawDebugCircle(
			GetWorld(), 
			FMatrix(Up, Right, Forward, Center), 
			View_Range, 
			32, 
			FColor::Green, 
			true, 
			-1.f, 
			0, 
			2.0f, 
			false 
		);

		// 시야각(부채꼴) 선 그리기 (이 부분은 기존과 동일)
		FVector LeftDir = Forward.RotateAngleAxis(-View_Angle * 0.5f, FVector::UpVector);
		FVector RightDir = Forward.RotateAngleAxis(View_Angle * 0.5f, FVector::UpVector);

		DrawDebugLine(GetWorld(), Center, Center + LeftDir * View_Range, FColor::Green, true, -1.f, 0, 2.0f);
		DrawDebugLine(GetWorld(), Center, Center + RightDir * View_Range, FColor::Green, true, -1.f, 0, 2.0f);
	}
#endif
}

void ABaseZombie::HandleStateChange(EMonsterMainState NewState)
{
	if (!AudioLoopComponent || !MonsterData) return;
	
	if (NewState == EMonsterMainState::Idle)
	{
		if (MonsterData->IdleMontage)
		{
			PlayAnimM(MonsterData->IdleMontage);
		}
	}
	
	if (const FMonsterStateSettings* Settings = MonsterData->StateConfigMap.Find(NewState))
	{
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = MonsterData->StateConfigMap[NewState].MovementSpeed;
		}
		
		if (Settings->StateLoopSound && AudioLoopComponent->GetSound() != Settings->StateLoopSound)
		{
			AudioLoopComponent->SetSound(Settings->StateLoopSound);
			AudioLoopComponent->Play();
		}
	}
}
