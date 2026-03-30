#include "Level_YC/BreakableWall.h"
#include "Field/FieldSystemTypes.h"  
#include "Field/FieldSystemObjects.h" 
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"    
#include "WardGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h" 
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
# include "Chaos/ChaosEngineInterface.h"


ABreakableWall::ABreakableWall()
{
    PrimaryActorTick.bCanEverTick = false;

    GCComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GCComponent"));
    RootComponent = GCComponent;
    GCComponent->SetNotifyBreaks(true);
    FieldSystemComponent = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystem"));
}

void ABreakableWall::ExecuteWallDestruction(FVector ImpactLocation)
{
    if (bIsAlreadyBroken || !GCComponent || !FieldSystemComponent) return;
    bIsAlreadyBroken = true;

    UFieldSystemMetaDataFilter* FilterMeta = NewObject<UFieldSystemMetaDataFilter>(this);

    if (FilterMeta)
    {
        FilterMeta->SetMetaDataFilterType(
            EFieldFilterType::Field_Filter_All,           
            EFieldObjectType::Field_Object_Destruction,    
            EFieldPositionType::Field_Position_CenterOfMass
        );
    }

    URadialFalloff* RadialFalloff = NewObject<URadialFalloff>(this);
    RadialFalloff->SetRadialFalloff(StrainMagnitude, 0.f, 1.f, 0.f, DamageRadius, ImpactLocation, (EFieldFalloffType)0);

    URadialVector* RadialVec = NewObject<URadialVector>(this);
    RadialVec->SetRadialVector(PushMagnitude, ImpactLocation);

    UUniformVector* UniformVec = NewObject<UUniformVector>(this);
    UniformVec->SetUniformVector(2000.f, ImpactDirection);

    
    
    FieldSystemComponent->ApplyPhysicsField(true, EFieldPhysicsType::Field_ExternalClusterStrain, nullptr, RadialFalloff);
    FieldSystemComponent->ApplyPhysicsField(true, EFieldPhysicsType::Field_LinearVelocity, FilterMeta, RadialVec);
    FieldSystemComponent->ApplyPhysicsField(true, EFieldPhysicsType::Field_LinearVelocity, FilterMeta, UniformVec);

   
    GCComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GCComponent->SetCollisionResponseToAllChannels(ECR_Block);
    GCComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); 
    GCComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);
    GCComponent->SetCanEverAffectNavigation(false);
    

    if (DustEffect)
    {
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            DustEffect,
            GetActorLocation()
        );

        if (NiagaraComp)
        {
            FVector WallSize = GCComponent->Bounds.BoxExtent * 2.0f;
            NiagaraComp->SetVariableVec3(FName("User.WallSize"), WallSize);
            NiagaraComp->SetVariableObject(FName("User.User_ChaosDI"), GCComponent);
        }
    }

    UWardGameInstanceSubsystem* WGInstanceSubsys = GetGameInstance()->GetSubsystem<UWardGameInstanceSubsystem>();
    
    if (BreakSound && !WGInstanceSubsys->IsBossDefeated(FName(TEXT("Huton"))))
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            BreakSound,
            GetActorLocation(),
            1.0f, 
            1.0f, 
            0.0f, 
            nullptr 
        );
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BreakableWall: BreakSound is missing!"));
    }
}

void ABreakableWall::BeginPlay()
{
    Super::BeginPlay();
    
   
}
