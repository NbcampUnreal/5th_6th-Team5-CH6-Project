#include "Character/Data/Combat/CharacterCombatData.h"

FPrimaryAssetId UCharacterCombatData::GetPrimaryAssetId() const
{
    return FPrimaryAssetId("CharacterData", GetFName());
}
