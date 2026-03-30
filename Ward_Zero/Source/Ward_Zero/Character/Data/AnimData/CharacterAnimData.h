#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Enum/CharacterType.h"
#include "CharacterAnimData.generated.h"

class UAnimMontage;

UENUM(BlueprintType)
enum class EWeaponAnimType : uint8
{
    Unarmed,
    Pistol,
    SMG
};

UCLASS(BlueprintType)
class WARD_ZERO_API UCharacterAnimData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId("CharacterData", GetFName()); }

    //// 피격 몽타주 (방향별)
    UPROPERTY(EditAnywhere, Category = "Montage|Hit")
    TMap<EPlayerHitDirection, TObjectPtr<UAnimMontage>> UnarmedHitMontages;

    UPROPERTY(EditAnywhere, Category = "Montage|Hit")
    TMap<EPlayerHitDirection, TObjectPtr<UAnimMontage>> PistolHitMontages;

    UPROPERTY(EditAnywhere, Category = "Montage|Hit")
    TMap<EPlayerHitDirection, TObjectPtr<UAnimMontage>> SMGHitMontages;

    //사망 몽터주 (방향별)
    UPROPERTY(EditAnywhere, Category = "Montage|Death")
    TMap<EPlayerHitDirection, TObjectPtr<UAnimMontage>> DeathMontages;

    // 기타 공통 몽타주
    UPROPERTY(EditAnywhere, Category = "Montage|Common")
    TObjectPtr<UAnimMontage> QuickTurn180;

    UPROPERTY(EditAnywhere, Category = "Montage|Common")
    TObjectPtr<UAnimMontage> FallLanding;

    UPROPERTY(EditAnywhere, Category = "Montage|Interaction")
    TObjectPtr<UAnimMontage> DoorPushOpenMontage;

    UPROPERTY(EditAnywhere, Category = "Montage|Interaction")
    TObjectPtr<UAnimMontage> DoorPullOpenMontage;

    UPROPERTY(EditAnywhere, Category = "Montage|Interaction")
    TObjectPtr<UAnimMontage> LeverMontage; // 레버 내리는 몽타주

    UPROPERTY(EditAnywhere, Category = "Montage|Action")
    TObjectPtr<UAnimMontage> HealMontage;

    UPROPERTY(EditAnywhere, Category = "Montage|Action")
    TObjectPtr<UAnimMontage> PickupLowMontage;

    UPROPERTY(EditAnywhere, Category = "Montage|Action")
    TObjectPtr<UAnimMontage> PickupHighMontage;

    UPROPERTY(EditAnywhere, Category = "Turn")
    TMap<EWeaponLayerType, UAnimMontage*> TurnLeft90Montages;

    UPROPERTY(EditAnywhere, Category = "Turn")
    TMap<EWeaponLayerType, UAnimMontage*> TurnRight90Montages;

    UPROPERTY(EditAnywhere, Category = "Interaction|Sound")
    TObjectPtr<USoundBase> SlidingDoorSound;

    UPROPERTY(EditAnywhere, Category = "Interaction|Sound")
    TObjectPtr<USoundBase> DoubleDoorSound;

    UPROPERTY(EditAnywhere, Category = "Montage|Interaction")
    TObjectPtr<UAnimMontage> LockedDoorMontage;  

    UPROPERTY(EditAnywhere, Category = "Montage|Interaction")
    TObjectPtr<UAnimMontage> ButtonPressMontage;
};
