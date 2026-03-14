#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerAnimInterface.generated.h"

class UPlayerCombatComponent;

UINTERFACE(MinimalAPI)
class UPlayerAnimInterface : public UInterface
{
    GENERATED_BODY()
};

class WARD_ZERO_API IPlayerAnimInterface
{
    GENERATED_BODY()

public:
    virtual bool GetIsRunning() const = 0;
    virtual bool GetIsPistolEquipped() const = 0;
    virtual bool GetIsCrouching() const = 0;
    virtual bool GetIsGround() const = 0;
    virtual bool GetIsQuickTurning() const = 0;
    virtual int32 GetTurnIndex() const = 0;
    virtual bool IsEquipping() const = 0;
    virtual FVector GetHandIKTargetLoc() const = 0;
    virtual bool GetIsAiming() const = 0;
    virtual void SetIsQuickTurning(bool bIsTurning) = 0;
    virtual class USkeletalMeshComponent* GetEquippedWeaponMesh() = 0;
    virtual class AWeapon* GetEquippedWeapon() = 0;
    virtual bool GetIsReloading() const = 0;
    virtual bool GetIsUseFlashLight() const = 0;
    virtual bool GetIsSMGEquipped() const = 0;
    virtual int32 GetCurrentWeaponIndex() const = 0;
    virtual float GetAimPitch() const = 0;
    virtual float GetAimYaw() const = 0;
    virtual bool IsFiring() const = 0;
    virtual float GetCurrSpread() const = 0;
    virtual UPlayerCombatComponent* GetCombatComp() const = 0;
    virtual bool GetbIsWeaponDrawn() const = 0;
    virtual bool GetIsInjured() const = 0;
};