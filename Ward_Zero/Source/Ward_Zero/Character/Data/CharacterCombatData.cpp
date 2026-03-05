#include "Character/Data/CharacterCombatData.h"

FPrimaryAssetId UCharacterCombatData::GetPrimaryAssetId() const
{
    return FPrimaryAssetId("CharacterData", GetFName());
}
