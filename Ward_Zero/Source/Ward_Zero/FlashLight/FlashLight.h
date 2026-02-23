#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlashLight.generated.h"

class USpotLightComponent;

UCLASS()
class WARD_ZERO_API AFlashLight : public AActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USpotLightComponent> SpotLight;
	
public:	
	AFlashLight();

};
