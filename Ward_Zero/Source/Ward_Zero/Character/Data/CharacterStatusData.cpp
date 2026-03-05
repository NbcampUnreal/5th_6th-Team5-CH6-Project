#include "Character/Data/CharacterStatusData.h"

FPrimaryAssetId UCharacterStatusData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("CharacterData", GetFName());
}
