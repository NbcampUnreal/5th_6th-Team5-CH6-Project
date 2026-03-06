#include "FlashLight/FlashLight.h"
#include "Components/SpotLightComponent.h"
#include "FlashLight/Data/FlashLightData.h"

AFlashLight::AFlashLight()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	SpotLight->SetupAttachment(RootComponent);

	SpotLight->SetRelativeLocation(FVector(0.0f, 10.0f, 0.0f));
	SpotLight->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
}

void AFlashLight::InitializeLight(UFlashLightData* Data)
{
	if (Data && SpotLight)
	{
		SpotLight->Intensity = Data->Intensity;
		SpotLight->OuterConeAngle = Data->OuterConeAngle;
		SpotLight->AttenuationRadius = Data->AttenuationRadius;
	}
}

