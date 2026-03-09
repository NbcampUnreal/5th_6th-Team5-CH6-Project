#include "Character/Data/Status/CharacterStatusData.h"

FPrimaryAssetId UCharacterStatusData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("CharacterData", GetFName());
}
