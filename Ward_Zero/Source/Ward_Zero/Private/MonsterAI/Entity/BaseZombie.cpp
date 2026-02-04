// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAI/Entity/BaseZombie.h"

#include "UnrealEdGlobals.h"
#include "Public/MonsterAI/Data/MonsterDataAsset.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Editor/UnrealEdEngine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MonsterAI/Component/StatusComponent.h"


// Sets default values
ABaseZombie::ABaseZombie()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StatusComponent  = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	AudioLoopComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioLoopComponent"));
	AudioLoopComponent->SetupAttachment(GetRootComponent());
	AudioLoopComponent->SetAutoActivate(false);
	
	// visualize capsule comp
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetHiddenInGame(false);
	}
}

// Called when the game starts or when spawned
void ABaseZombie::BeginPlay()
{
	Super::BeginPlay();
	if (StatusComponent && MonsterData)
	{
		StatusComponent->InitData(MonsterData);
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = StatusComponent->GetBaseSpeed();

		}
	}
}

// Called every frame
void ABaseZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
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
        
		
		if (MonsterData->AnimBPClass)
		{
			GetMesh()->SetAnimInstanceClass(MonsterData->AnimBPClass);
		}

		
		GetMesh()->SetRelativeScale3D(MonsterData->MeshScale);
        
		
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = MonsterData->BaseSpeed;
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