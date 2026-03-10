#include "Character/Noise/NoiseFucLibrary/PlayerNoise.h"
#include "Perception/AISense_Hearing.h"

void UPlayerNoise::ReportNoise
(
	UObject* WorldContextObject, 
	AActor* Instigator, 
	FVector Location, 
	float Loudness, 
	float Range, 
	FName Tag)
{
    if (!WorldContextObject || !Instigator) return;

    // 언리얼 기본 청각 센서에 데이터 전송
    UAISense_Hearing::ReportNoiseEvent(
        WorldContextObject,
        Location,
        Loudness,
        Instigator,
        Range,
        Tag
    );
}
