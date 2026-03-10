#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PlayerNoise.generated.h"

UCLASS()
class WARD_ZERO_API UPlayerNoise : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "AI | Noise", meta = (WorldContext = "WorldContextObject"))
	static void ReportNoise
	(
		UObject* WorldContextObject, // 월드
		AActor* Instigator, // 소리 주체
		FVector Location,  // 발생 위치
		float Loudness,    // 들리는 거리
		float Range, FName Tag // 태그
	);
};
