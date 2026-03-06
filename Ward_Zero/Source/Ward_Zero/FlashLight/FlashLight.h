#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlashLight.generated.h"

class USpotLightComponent;
class UFlashLightData;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|LightSettings", meta = (ClampMin = "0.0"))
	float LightIntensity = 5000.0f; // 밝기

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|LightSettings", meta = (ClampMin = "0.0", ClampMax = "80.0"))
	float LightOuterConeAngle = 30.0f; // 빛이 퍼지는 전체 각도

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|LightSettings", meta = (ClampMin = "0.0", ClampMax = "80.0"))
	float LightInnerConeAngle = 10.0f; // 중심부의 밝은 각도

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flashlight|LightSettings", meta = (ClampMin = "0.0"))
	float LightAttenuationRadius = 200.0f; // 빛이 도달하는 거리 

public:
	void InitializeLight(UFlashLightData* Data);
};
