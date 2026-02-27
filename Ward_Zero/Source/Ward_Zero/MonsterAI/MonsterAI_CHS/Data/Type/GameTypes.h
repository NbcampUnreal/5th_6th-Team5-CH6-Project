#pragma once

UENUM(BlueprintType)
enum class EHitDirection : uint8
{
	Front,
	Back,
	Left,
	Right,
};

UENUM(BlueprintType)
enum class EHitPart: uint8
{
	Head,
	Body,
	LegLeft,
	LegRight,
};

UENUM(BlueprintType)
enum class EInteractableObject: uint8
{
	Door,
	Window,
};

namespace GPTags
{
	const FName Head = FName("Combat.HitPart.Head");
	const FName Body = FName("Combat.HitPart.Body");
	const FName LegLeft = FName("Combat.HitPart.Leg.Left");
	const FName LegRight = FName("Combat.HitPart.Leg.Right");
	const FName Door = FName("Interaction.Object.Door");
	const FName Window = FName("Interaction.Object.Window");
}