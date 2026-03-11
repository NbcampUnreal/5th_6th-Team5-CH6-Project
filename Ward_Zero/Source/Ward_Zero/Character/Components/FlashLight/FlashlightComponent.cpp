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
        // 컴포넌트 및 인터페이스 캐싱
        CachedCombatComp = Owner->FindComponentByClass<UPlayerCombatComponent>();

		// 1. 가슴 라이트(BodyRunLight) 동적 생성 및 설정
		// (참고: 두 번째 인자는 언리얼 내부용 컴포넌트 이름입니다. 소켓 이름이 아니어도 됩니다.)
		BodyRunLight = NewObject<USpotLightComponent>(Owner, TEXT("BodyRunLightComp"));
		if (BodyRunLight)
		{
			BodyRunLight->RegisterComponent();
			BodyRunLight->AttachToComponent(Owner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("BodyLightSocket"));

			// ★ 회전값만 월드 기준(Absolute)으로 설정하여 애니메이션의 흔들림을 무시합니다.
			// 위치는 소켓을 따라가지만, 빛이 가리키는 방향은 캐릭터의 기본 방향을 유지합니다.
			BodyRunLight->SetUsingAbsoluteRotation(true);

			BodyRunLight->SetRelativeLocation(FVector::ZeroVector);
			BodyRunLight->SetRelativeRotation(FRotator::ZeroRotator);

			BodyRunLight->SetVisibility(false);
			BodyRunLight->SetCastShadows(false);
		}

		// 실제 손전등 액터(FlashLightActor) 월드에 스폰 
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
    if (!CachedCombatComp || !BodyRunLight) return;

    APrototypeCharacter* Player = Cast<APrototypeCharacter>(GetOwner());
    if (!Player) return;

    IPlayerAnimInterface* AnimIF = Cast<IPlayerAnimInterface>(Player);
    if (!AnimIF) return;

    // 회전 및 초기화
    BodyRunLight->SetWorldRotation(Player->GetActorRotation());
    BodyRunLight->SetVisibility(false);

    AWeapon* CurrentWeapon = Player->GetEquippedWeapon();
    if (CachedCombatComp)
    {
        TArray<AWeapon*> AllWeapons;
        if (CachedCombatComp->PistolWeapon) AllWeapons.Add(CachedCombatComp->PistolWeapon);
        if (CachedCombatComp->SMGWeapon) AllWeapons.Add(CachedCombatComp->SMGWeapon);
        for (AWeapon* Weapon : AllWeapons)
        {
            if (Weapon && Weapon->SMGSpotLight) Weapon->SMGSpotLight->SetVisibility(false);
        }
    }

    // 전등 OFF 상태 (Ambient)
    if (!bIsUseFlashlight)
    {
        if (FlashLightActor) FlashLightActor->SetActorHiddenInGame(true);

        if (DefaultFlashlightData)
        {
            BodyRunLight->SetVisibility(true);
            BodyRunLight->SetIntensity(DefaultFlashlightData->AmbientIntensity);
            BodyRunLight->SetAttenuationRadius(DefaultFlashlightData->AmbientRadius);
            BodyRunLight->SetOuterConeAngle(75.f); 
        }
        return; 
    }

    // 손전등 ON 상태: 무기별 데이터 에셋 결정
    UFlashLightData* TargetData = (CurrentWeapon && CurrentWeapon->WeaponData && CurrentWeapon->WeaponData->FlashlightSettings)
        ? CurrentWeapon->WeaponData->FlashlightSettings : DefaultFlashlightData;

    if (!TargetData) return;

    bool bIsRunning = AnimIF->GetIsRunning();
    bool bIsUnarmed = !CachedCombatComp->IsWeaponDrawn();

    USpotLightComponent* ActiveSpot = nullptr;
    float BaseIntensity = TargetData->Intensity;
    float BaseOuterAngle = TargetData->OuterConeAngle;

    if (bIsRunning)
    {
        // [상황 A] 달리는 중
        BodyRunLight->SetVisibility(true);

        BaseIntensity = TargetData->Intensity * TargetData->SprintIntensityMultiplier;
        float SprintRadius = TargetData->AttenuationRadius * TargetData->SprintRadiusMultiplier;

        BodyRunLight->SetIntensity(BaseIntensity);
        BodyRunLight->SetAttenuationRadius(SprintRadius);
        BodyRunLight->SetOuterConeAngle(BaseOuterAngle);
        BodyRunLight->SetInnerConeAngle(BaseOuterAngle * TargetData->InnerConeRatio);
        BodyRunLight->SetVolumetricScatteringIntensity(TargetData->VolumetricScatteringIntensity);
        BodyRunLight->SetCastShadows(TargetData->bCastShadows);

        ActiveSpot = BodyRunLight;

        if (FlashLightActor)
        {
            FlashLightActor->SetActorHiddenInGame(!bIsUnarmed);
            if (auto* Spot = FlashLightActor->FindComponentByClass<USpotLightComponent>()) Spot->SetVisibility(false);
        }
    }
    else
    {
        // 걷기 또는 조준 시
        if (AnimIF->GetIsSMGEquipped() && CurrentWeapon && CurrentWeapon->SMGSpotLight)
        {
            CurrentWeapon->SMGSpotLight->SetVisibility(true);
            CurrentWeapon->SMGSpotLight->SetIntensity(BaseIntensity);
            CurrentWeapon->SMGSpotLight->SetOuterConeAngle(BaseOuterAngle);
            CurrentWeapon->SMGSpotLight->SetInnerConeAngle(BaseOuterAngle * TargetData->InnerConeRatio);
            CurrentWeapon->SMGSpotLight->SetAttenuationRadius(TargetData->AttenuationRadius);
            CurrentWeapon->SMGSpotLight->SetCastShadows(TargetData->bCastShadows);

            ActiveSpot = CurrentWeapon->SMGSpotLight;
        }
        else if (FlashLightActor)
        {
            bool bShouldHide = AnimIF->GetIsReloading() || AnimIF->IsEquipping();
            if (!bShouldHide)
            {
                if (bIsUnarmed) bShouldHide = false;
                FName SocketName = AnimIF->GetIsPistolEquipped() ? TEXT("FlashLightSocket_Pistol") : TEXT("FlashLightSocket_Normal");
                FlashLightActor->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
                FlashLightActor->SetActorRelativeLocation(FVector::ZeroVector);
                FlashLightActor->SetActorRelativeRotation(FRotator::ZeroRotator);
                FlashLightActor->SetActorHiddenInGame(false);

                FlashLightActor->InitializeLight(TargetData);

                if (auto* Spot = FlashLightActor->FindComponentByClass<USpotLightComponent>())
                {
                    Spot->SetVisibility(true);
                    Spot->SetIntensity(BaseIntensity);
                    Spot->SetInnerConeAngle(BaseOuterAngle * TargetData->InnerConeRatio);
                    Spot->SetCastShadows(TargetData->bCastShadows);

                    ActiveSpot = Spot;
                }
            }
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
    FVector Start = Light->GetComponentLocation();
    FVector End = Start + (Light->GetForwardVector() * MaxDist);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());
    if (FlashLightActor) Params.AddIgnoredActor(FlashLightActor);

    // 라인 트레이스로 벽까지의 거리 측정
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        // 0(벽에 붙음) ~ 1(멀리 있음) 사이의 값 반환
        return FMath::Clamp(Hit.Distance / MaxDist, 0.0f, 1.0f);
    }

    return 1.0f;
}

void UFlashlightComponent::ToggleFlashlight()
{
	APrototypeCharacter* Player = Cast<APrototypeCharacter>(GetOwner());
	if (!Player) return;

	bIsUseFlashlight = !bIsUseFlashlight;

	if (Player->GetCurrentWeaponIndex() == 0) // 0이 Unarmed라고 가정 (혹은 !Player->GetIsWeaponDrawn())
	{
		if (bIsUseFlashlight && RaiseLightMontage)
			Player->PlayAnimMontage(RaiseLightMontage);
		else if (!bIsUseFlashlight && LowerLightMontage)
			Player->PlayAnimMontage(LowerLightMontage);
	}
	else
	{
		// 무기를 들고 있을 때는 몽타주를 재생하지 않습니다.
		// 대신 필요하다면 여기서 소리만 재생하거나 즉시 상태 업데이트를 수행합니다.
	}
	UpdateFlashlight(0.0f);
}