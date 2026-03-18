#include "Character/Components/FlashLight/FlashlightComponent.h"
#include "FlashLight/FlashLight.h"
#include "FlashLight/Data/FlashLightData.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Components/Combat/PlayerCombatComponent.h"
#include "Character/Prototype_Character/PrototypeCharacter.h"
#include "Character/Animation/Interface/PlayerAnimInterface.h"
#include "Character/Components/Status/PlayerStatusComponent.h"
#include "GameFramework/Character.h"
#include "Weapon/Weapon.h"
#include "Weapon/Data/WeaponData.h"

UFlashlightComponent::UFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFlashlightComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickInterval(0.1f);

	if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
	{
		CachedCombatComp = Owner->FindComponentByClass<UPlayerCombatComponent>();

		if (FlashLightClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Owner;
			SpawnParams.Instigator = Owner;

			FlashLightActor = GetWorld()->SpawnActor<AFlashLight>(FlashLightClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (FlashLightActor)
			{
				FlashLightActor->SetActorEnableCollision(false);
				FlashLightActor->SetActorHiddenInGame(true);
			}
		}
	}
}

void UFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFlashlight(DeltaTime);
}

void UFlashlightComponent::SetFlashlightOff()
{
	bIsUseFlashlight = false;
	UpdateFlashlight(0.0f);
}

void UFlashlightComponent::UpdateFlashlight(float DeltaTime)
{
	if (!CachedCombatComp || !FlashLightActor) return;

	APrototypeCharacter* Player = Cast<APrototypeCharacter>(GetOwner());
	if (!Player) return;

	IPlayerAnimInterface* AnimIF = Cast<IPlayerAnimInterface>(Player);
	if (!AnimIF) return;

	USpotLightComponent* FlashLightSpot = FlashLightActor->FindComponentByClass<USpotLightComponent>();
	UStaticMeshComponent* FlashLightMesh = FlashLightActor->FindComponentByClass<UStaticMeshComponent>();
	if (!FlashLightSpot) return;

	AWeapon* CurrentWeapon = Player->GetEquippedWeapon();

	if (LastEquippedWeapon && LastEquippedWeapon != CurrentWeapon)
	{
		if (LastEquippedWeapon->SMGSpotLight)
		{
			LastEquippedWeapon->SMGSpotLight->SetVisibility(false);
		}
		if (LastEquippedWeapon->WeaponMesh)
		{
			LastEquippedWeapon->WeaponMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), 0.0f);
		}
	} 
	LastEquippedWeapon = CurrentWeapon;
	if (CurrentWeapon)
	{
		if (CurrentWeapon->SMGSpotLight)
		{
			CurrentWeapon->SMGSpotLight->SetVisibility(false);
		}
		if (CurrentWeapon->WeaponMesh)
		{
			CurrentWeapon->WeaponMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), 0.0f);
		}
	}

	float TargetEmissive = bIsUseFlashlight ? 50.0f : 0.0f;

	if (!bIsUseFlashlight)
	{
		FlashLightActor->SetActorHiddenInGame(true);
		FlashLightSpot->SetVisibility(false);
		if (FlashLightMesh) FlashLightMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), 0.0f);
		return;
	}

	UFlashLightData* TargetData = (CurrentWeapon && CurrentWeapon->WeaponData && CurrentWeapon->WeaponData->FlashlightSettings)
		? CurrentWeapon->WeaponData->FlashlightSettings : DefaultFlashlightData;

	if (!TargetData) return;

	bool bIsRunning = AnimIF->GetIsRunning();
	bool bIsUnarmed = !CachedCombatComp->IsWeaponDrawn();

	USpotLightComponent* ActiveSpot = nullptr;
	float BaseIntensity = TargetData->Intensity;
	float BaseOuterAngle = TargetData->OuterConeAngle;

	// 1. 달리는 중
	if (bIsRunning)
	{
		FName RunSocket = bIsUnarmed ? TEXT("FlashLightSocket_Normal") : TEXT("BodyLightSocket");
		FlashLightActor->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RunSocket);

		FlashLightActor->SetActorHiddenInGame(false);
		if (FlashLightMesh)
		{
			FlashLightMesh->SetVisibility(bIsUnarmed);
			FlashLightMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), bIsUnarmed ? TargetEmissive : 0.0f);
		}

		if (AnimIF->GetIsSMGEquipped() && CurrentWeapon && CurrentWeapon->WeaponMesh)
		{
			CurrentWeapon->WeaponMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), TargetEmissive);
		}

		FlashLightSpot->SetUsingAbsoluteRotation(true);
		FlashLightSpot->SetWorldRotation(Player->GetActorRotation());
		FlashLightSpot->SetVisibility(true);

		FlashLightSpot->SetIntensity(BaseIntensity * TargetData->SprintIntensityMultiplier);
		FlashLightSpot->SetAttenuationRadius(TargetData->AttenuationRadius * TargetData->SprintRadiusMultiplier);
		FlashLightSpot->SetOuterConeAngle(BaseOuterAngle);
		FlashLightSpot->SetInnerConeAngle(BaseOuterAngle * TargetData->InnerConeRatio);

		ActiveSpot = FlashLightSpot;
	}
	// 2. 걷기 또는 조준 시
	else
	{
		// SMG 케이스
		if (AnimIF->GetIsSMGEquipped() && CurrentWeapon && CurrentWeapon->SMGSpotLight)
		{
			FlashLightActor->SetActorHiddenInGame(true);
			FlashLightSpot->SetVisibility(false);
			if (FlashLightMesh) FlashLightMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), 0.0f);

			CurrentWeapon->SMGSpotLight->SetUsingAbsoluteRotation(true);
			CurrentWeapon->SMGSpotLight->SetWorldRotation(Player->GetControlRotation());
			CurrentWeapon->SMGSpotLight->SetVisibility(true);

			CurrentWeapon->SMGSpotLight->SetIntensity(BaseIntensity);
			CurrentWeapon->SMGSpotLight->SetOuterConeAngle(BaseOuterAngle);

			CurrentWeapon->SMGSpotLight->SetAttenuationRadius(TargetData->AttenuationRadius);
			CurrentWeapon->SMGSpotLight->SetInnerConeAngle(BaseOuterAngle * TargetData->InnerConeRatio);
			CurrentWeapon->SMGSpotLight->SetCastShadows(TargetData->bCastShadows);

			if (CurrentWeapon->WeaponMesh)
			{
				CurrentWeapon->WeaponMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), TargetEmissive);
			}

			ActiveSpot = CurrentWeapon->SMGSpotLight;
		}
		// 권총 또는 비무장 케이스
		else
		{
			bool bIsPistol = AnimIF->GetIsPistolEquipped();
			bool bIsAiming = AnimIF->GetIsAiming();

			bool bShouldHide = (AnimIF->GetIsReloading() || AnimIF->IsEquipping()) && !bIsUnarmed;
			if (bIsUnarmed) bShouldHide = false;

			FName SocketName;

			if (bIsPistol && !bIsAiming)
			{
				// 권총 비조준 시: 어깨에 붙이고 메쉬는 숨김 (BodyLight 모드)
				SocketName = TEXT("BodyLightSocket");
				if (FlashLightMesh) FlashLightMesh->SetVisibility(false);

				FlashLightSpot->SetUsingAbsoluteRotation(true);
				FlashLightSpot->SetWorldRotation(Player->GetActorRotation()); // 몸 방향 고정
			}
			else if (bIsPistol && bIsAiming)
			{
				// 권총 조준 시: 권총 손 소켓에 붙이고 메쉬 보임
				SocketName = TEXT("FlashLightSocket_Pistol");
				if (FlashLightMesh) FlashLightMesh->SetVisibility(!bShouldHide);

				FlashLightSpot->SetUsingAbsoluteRotation(true);
				FlashLightSpot->SetWorldRotation(Player->GetControlRotation()); // 카메라 방향 고정
			}
			else
			{
				// 비무장(Unarmed)일 때
				SocketName = TEXT("FlashLightSocket_Normal");
				if (FlashLightMesh) FlashLightMesh->SetVisibility(!bShouldHide);

				FlashLightSpot->SetUsingAbsoluteRotation(true);
				FlashLightSpot->SetWorldRotation(Player->GetControlRotation());
			}

			FlashLightActor->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
			FlashLightActor->SetActorRelativeLocation(FVector::ZeroVector);
			FlashLightActor->SetActorRelativeRotation(FRotator::ZeroRotator);

			FlashLightActor->SetActorHiddenInGame(false);
			FlashLightSpot->SetVisibility(!bShouldHide);

			if (!bShouldHide)
			{
				FlashLightSpot->SetIntensity(BaseIntensity);
				FlashLightSpot->SetOuterConeAngle(BaseOuterAngle);
				FlashLightSpot->SetInnerConeAngle(BaseOuterAngle * TargetData->InnerConeRatio);
				FlashLightSpot->SetAttenuationRadius(TargetData->AttenuationRadius);
				FlashLightSpot->SetCastShadows(TargetData->bCastShadows);
			}

			ActiveSpot = bShouldHide ? nullptr : FlashLightSpot;
		}
	}

	// 3. 동적 초점(Dynamic Focus) 적용 
	if (ActiveSpot && TargetData->bEnableDynamicFocus)
	{
		float FocusAlpha = CalculateFocusAlpha(ActiveSpot, TargetData->MaxFocusDistance);
		float DynamicOuter = FMath::Lerp(TargetData->MinOuterConeAngle, BaseOuterAngle, FocusAlpha);
		ActiveSpot->SetOuterConeAngle(DynamicOuter);
		ActiveSpot->SetInnerConeAngle(DynamicOuter * TargetData->InnerConeRatio);

		float DynamicIntensity = BaseIntensity * FMath::Lerp(TargetData->CloseRangeIntensityMultiplier, 1.0f, FocusAlpha);
		ActiveSpot->SetIntensity(DynamicIntensity);
	}
}

float UFlashlightComponent::CalculateFocusAlpha(USpotLightComponent* Light, float MaxDist)
{
	if (!Light) return 1.0f;

	FHitResult Hit;
	FVector Start = Light->GetComponentLocation() + (Light->GetForwardVector() * 15.0f);
	FVector End = Start + (Light->GetForwardVector() * MaxDist);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	if (FlashLightActor) Params.AddIgnoredActor(FlashLightActor);

	APrototypeCharacter* Player = Cast<APrototypeCharacter>(GetOwner());
	if (Player && Player->GetEquippedWeapon())
	{
		Params.AddIgnoredActor(Player->GetEquippedWeapon());
	}

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return FMath::Clamp(Hit.Distance / MaxDist, 0.0f, 1.0f);
	}

	return 1.0f;
}

void UFlashlightComponent::ToggleFlashlight()
{
	APrototypeCharacter* Player = Cast<APrototypeCharacter>(GetOwner());
	if (!Player) return;

	bIsUseFlashlight = !bIsUseFlashlight;

	if (Player->GetCurrentWeaponIndex() == 0)
	{
		if (bIsUseFlashlight && RaiseLightMontage)
			Player->PlayAnimMontage(RaiseLightMontage);
		else if (!bIsUseFlashlight && LowerLightMontage)
			Player->PlayAnimMontage(LowerLightMontage);
	}
	UpdateFlashlight(0.0f);
}