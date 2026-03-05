#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "CharacterAnimData.generated.h"

class UAnimMontage;

UCLASS(BlueprintType)
class WARD_ZERO_API UCharacterAnimData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId("CharacterData", GetFName()); }

    // 피격 몽타주 (방향별)
    UPROPERTY(EditAnywhere, Category = "Montage|Hit")
    TMap<EPlayerHitDirection, TObjectPtr<UAnimMontage>> HitMontages;

    //사망 몽터주 (방향별)
    UPROPERTY(EditAnywhere, Category = "Montage|Death")
    TMap<EPlayerHitDirection, TObjectPtr<UAnimMontage>> DeathMontages;

    // 기타 공통 몽타주
    UPROPERTY(EditAnywhere, Category = "Montage|Common")
    TObjectPtr<UAnimMontage> QuickTurn180;

    UPROPERTY(EditAnywhere, Category = "Montage|Common")
    TObjectPtr<UAnimMontage> FallLanding;

};
