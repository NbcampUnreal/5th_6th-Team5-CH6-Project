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
	SetComponentTickInterval(0.0f);

	if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
	{
		CurrentLightRotation = Owner->GetActorRotation();
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

	// 무기 교체 시 이전 무기 라이트 정리 
	if (LastEquippedWeapon && LastEquippedWeapon != CurrentWeapon)
	{
		if (LastEquippedWeapon->SMGSpotLight) LastEquippedWeapon->SMGSpotLight->SetVisibility(false);
		if (LastEquippedWeapon->WeaponMesh) LastEquippedWeapon->WeaponMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), 0.0f);
	}
	LastEquippedWeapon = CurrentWeapon;

	float TargetEmissive = bIsUseFlashlight ? 50.0f : 0.0f;

	// 손전등이 꺼져있을 때 처리
	if (!bIsUseFlashlight)
	{
		FlashLightActor->SetActorHiddenInGame(true);
		FlashLightSpot->SetVisibility(false);
		if (FlashLightMesh) FlashLightMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), 0.0f);
		if (CurrentWeapon && CurrentWeapon->SMGSpotLight) CurrentWeapon->SMGSpotLight->SetVisibility(false);
		return;
	}

	UFlashLightData* TargetData = (CurrentWeapon && CurrentWeapon->WeaponData && CurrentWeapon->WeaponData->FlashlightSettings)
		? CurrentWeapon->WeaponData->FlashlightSettings : DefaultFlashlightData;

	if (!TargetData) return;

	bool bIsRunning = AnimIF->GetIsRunning();
	bool bIsUnarmed = !CachedCombatComp->IsWeaponDrawn();
	bool bIsPistol = AnimIF->GetIsPistolEquipped();
	bool bIsAiming = AnimIF->GetIsAiming();

	USpotLightComponent* ActiveSpot = nullptr;
	float BaseIntensity = TargetData->Intensity;
	float BaseOuterAngle = TargetData->OuterConeAngle;


	FRotator TargetRotation;
	// 걷거나 비무장일 땐 카메라(Control) 방향, 무장/달리기 시엔 캐릭터 방향
	if (bIsRunning)
	{
		// 달릴 때는 캐릭터가 바라보는 방향 고정
		TargetRotation = Player->GetActorRotation();
	}
	else
	{
		// 마우스 방향(ControlRotation)을 가져옴
		FRotator ControlRot = Player->GetControlRotation();
		FRotator ActorRot = Player->GetActorRotation();

		// 캐릭터 정면 기준 카메라가 돌아간 차이값 계산
		FRotator DeltaRot = (ControlRot - ActorRot).GetNormalized();

		// 캐릭터 몸을 뚫지 않게 좌우/상하 각도 제한 (80도/60도)
		DeltaRot.Yaw = FMath::Clamp(DeltaRot.Yaw, -80.0f, 80.0f);
		DeltaRot.Pitch = FMath::Clamp(DeltaRot.Pitch, -60.0f, 60.0f);

		// 최종 타겟 = 몸 방향 + 제한된 카메라 차이값
		TargetRotation = ActorRot + DeltaRot;
	}
	if (DeltaTime <= 0.0f) CurrentLightRotation = TargetRotation;
	else CurrentLightRotation = FMath::RInterpTo(CurrentLightRotation, TargetRotation, DeltaTime, 20.0f);

	// 상태별 라이트 제어
	if (bIsRunning)
	{
		// 달릴 때는 무기 라이트 비활성화
		if (CurrentWeapon && CurrentWeapon->SMGSpotLight) CurrentWeapon->SMGSpotLight->SetVisibility(false);

		FName RunSocket = bIsUnarmed ? TEXT("FlashLightSocket_Normal") : TEXT("BodyLightSocket");
		FlashLightActor->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RunSocket);

		FlashLightActor->SetActorHiddenInGame(false);
		if (FlashLightMesh)
		{
			bool bShowMesh = bIsUnarmed;
			FlashLightMesh->SetVisibility(bShowMesh);
			FlashLightMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), bShowMesh ? TargetEmissive : 0.0f);
		}

		FlashLightSpot->SetUsingAbsoluteRotation(true);
		FlashLightSpot->SetWorldRotation(CurrentLightRotation);
		FlashLightSpot->SetVisibility(true);

		FlashLightSpot->SetIntensity(BaseIntensity * TargetData->SprintIntensityMultiplier);
		FlashLightSpot->SetAttenuationRadius(TargetData->AttenuationRadius * TargetData->SprintRadiusMultiplier);

		ActiveSpot = FlashLightSpot;
	}
	else
	{
		// SMG 상태이거나 "권총이면서 조준 중"일 때 무기의 라이트 사용
		if ((AnimIF->GetIsSMGEquipped() || (bIsPistol && bIsAiming)) && CurrentWeapon && CurrentWeapon->SMGSpotLight)
		{
			FlashLightActor->SetActorHiddenInGame(true);
			FlashLightSpot->SetVisibility(false);
			if (FlashLightMesh) FlashLightMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), 0.0f);

			// 무기 라이트 활성화
			CurrentWeapon->SMGSpotLight->SetVisibility(true);
			CurrentWeapon->SMGSpotLight->SetUsingAbsoluteRotation(true);
			CurrentWeapon->SMGSpotLight->SetWorldRotation(CurrentLightRotation);

			// 무기 데이터 에셋의 값 적용
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
		else // 권총 비조준 또는 비무장 (기존 손전등 액터 사용)
		{
			if (CurrentWeapon && CurrentWeapon->SMGSpotLight) CurrentWeapon->SMGSpotLight->SetVisibility(false);

			bool bShouldHide = (AnimIF->GetIsReloading() || AnimIF->IsEquipping()) && !bIsUnarmed;
			FName SocketName = (bIsPistol) ? TEXT("BodyLightSocket") : TEXT("FlashLightSocket_Normal");

			FlashLightActor->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
			FlashLightActor->SetActorHiddenInGame(false);
			if (FlashLightMesh)
			{
				bool bShowMesh = bIsUnarmed;
				FlashLightMesh->SetVisibility(bShowMesh && !bShouldHide);
				FlashLightMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), bShowMesh ? TargetEmissive : 0.0f);
			}

			FlashLightSpot->SetVisibility(!bShouldHide);
			if (!bShouldHide)
			{
				FlashLightSpot->SetUsingAbsoluteRotation(true);
				// 권총 비조준이면 몸 방향, 비무장이면 카메라 방향
				FlashLightSpot->SetWorldRotation(CurrentLightRotation);
				FlashLightSpot->SetIntensity(BaseIntensity);
				FlashLightSpot->SetOuterConeAngle(BaseOuterAngle);
				FlashLightSpot->SetAttenuationRadius(TargetData->AttenuationRadius);
			}

			ActiveSpot = bShouldHide ? nullptr : FlashLightSpot;
		}
	}

	// 동적 초점(Dynamic Focus) 적용 
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
    if (!Player || !CachedCombatComp) return;

    bIsUseFlashlight = !bIsUseFlashlight;

    if (!CachedCombatComp->IsWeaponDrawn())
    {
        UAnimMontage* MontageToPlay = bIsUseFlashlight ? RaiseLightMontage : LowerLightMontage;
        if (MontageToPlay)
        {
            Player->StopAnimMontage();
            Player->PlayAnimMontage(MontageToPlay);
        }
    }
    UpdateFlashlight(0.0f);
}