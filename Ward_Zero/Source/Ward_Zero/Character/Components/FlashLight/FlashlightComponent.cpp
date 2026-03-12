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
	if (!CachedCombatComp || !FlashLightActor) return;

	APrototypeCharacter* Player = Cast<APrototypeCharacter>(GetOwner());
	if (!Player) return;

	IPlayerAnimInterface* AnimIF = Cast<IPlayerAnimInterface>(Player);
	if (!AnimIF) return;

	// FlashLight 엑터 내부 SpotLight와 StaticMesh 가져오기
	USpotLightComponent* FlashLightSpot = FlashLightActor->FindComponentByClass<USpotLightComponent>();
	UStaticMeshComponent* FlashLightMesh = FlashLightActor->FindComponentByClass<UStaticMeshComponent>();
	if (!FlashLightSpot) return;

	AWeapon* CurrentWeapon = Player->GetEquippedWeapon();

	// SMG 라이트는 기본적으로 꺼둠
	if (CurrentWeapon && CurrentWeapon->SMGSpotLight)
	{
		CurrentWeapon->SMGSpotLight->SetVisibility(false);
	}

	// 전등 OFF 상태 (Ambient 빛 제거)
	if (!bIsUseFlashlight)
	{
		FlashLightActor->SetActorHiddenInGame(true);
		FlashLightSpot->SetVisibility(false);
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

	// 달리는 중
	if (bIsRunning)
	{
		// 가슴 소켓으로 손전등 이동
		FlashLightActor->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("BodyLightSocket"));

		// 액터 자체는 켜두어야 빛이 나옴, 대신 메쉬(모델링)만 투명하게 숨김
		FlashLightActor->SetActorHiddenInGame(false);
		if (FlashLightMesh) FlashLightMesh->SetVisibility(false);

		// 빛 회전을 몸통 절대 회전값으로 고정 (뛰어도 안 흔들림)
		FlashLightSpot->SetUsingAbsoluteRotation(true);
		FlashLightSpot->SetWorldRotation(Player->GetActorRotation());
		FlashLightSpot->SetVisibility(true);

		// 달리기 데이터 적용
		FlashLightSpot->SetIntensity(BaseIntensity * TargetData->SprintIntensityMultiplier);
		FlashLightSpot->SetAttenuationRadius(TargetData->AttenuationRadius * TargetData->SprintRadiusMultiplier);
		FlashLightSpot->SetOuterConeAngle(BaseOuterAngle);
		FlashLightSpot->SetInnerConeAngle(BaseOuterAngle * TargetData->InnerConeRatio);
		FlashLightSpot->SetVolumetricScatteringIntensity(TargetData->VolumetricScatteringIntensity);
		FlashLightSpot->SetCastShadows(TargetData->bCastShadows);

		ActiveSpot = FlashLightSpot;
	}
	// 걷기 또는 조준 시
	else
	{
		// SMG를 들고 있을 때
		if (AnimIF->GetIsSMGEquipped() && CurrentWeapon && CurrentWeapon->SMGSpotLight)
		{
			FlashLightActor->SetActorHiddenInGame(true);
			FlashLightSpot->SetVisibility(false);

			// SMG 라이트 흔들림 무시 + 카메라 정면으로 위치 고정 
			CurrentWeapon->SMGSpotLight->SetUsingAbsoluteRotation(true);
			CurrentWeapon->SMGSpotLight->SetWorldRotation(Player->GetControlRotation());

			CurrentWeapon->SMGSpotLight->SetVisibility(true);
			CurrentWeapon->SMGSpotLight->SetIntensity(BaseIntensity);
			CurrentWeapon->SMGSpotLight->SetOuterConeAngle(BaseOuterAngle);
			CurrentWeapon->SMGSpotLight->SetInnerConeAngle(BaseOuterAngle * TargetData->InnerConeRatio);
			CurrentWeapon->SMGSpotLight->SetAttenuationRadius(TargetData->AttenuationRadius);
			CurrentWeapon->SMGSpotLight->SetCastShadows(TargetData->bCastShadows);

			ActiveSpot = CurrentWeapon->SMGSpotLight;
		}
		// 권총이거나 비무장일 때
		else
		{
			bool bShouldHide = AnimIF->GetIsReloading() || AnimIF->IsEquipping();
			if (bIsUnarmed) bShouldHide = false;

			FName SocketName = AnimIF->GetIsPistolEquipped() ? TEXT("FlashLightSocket_Pistol") : TEXT("FlashLightSocket_Normal");

			// 손 소켓으로 이동
			FlashLightActor->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
			FlashLightActor->SetActorRelativeLocation(FVector::ZeroVector);
			FlashLightActor->SetActorRelativeRotation(FRotator::ZeroRotator);

			FlashLightActor->SetActorHiddenInGame(bShouldHide);

			// 메쉬 다시 보이게 복구
			if (FlashLightMesh) FlashLightMesh->SetVisibility(!bShouldHide);

			// 빛(SpotLight)은 손목의 흔들림을 무시하도록 절대 회전 켜기
			FlashLightSpot->SetUsingAbsoluteRotation(true);

			// SpotLight를 플레이어의 시야(카메라/크로스헤어) 정면으로 고정
			FlashLightSpot->SetWorldRotation(Player->GetControlRotation());

			if (AFlashLight* DefaultActor = FlashLightActor->GetClass()->GetDefaultObject<AFlashLight>())
			{
				if (USpotLightComponent* DefaultSpot = DefaultActor->FindComponentByClass<USpotLightComponent>())
				{
					FlashLightSpot->SetRelativeLocation(DefaultSpot->GetRelativeLocation());
				}
			}

			FlashLightSpot->SetVisibility(!bShouldHide);

			if (!bShouldHide)
			{
				FlashLightSpot->SetIntensity(BaseIntensity);
				FlashLightSpot->SetInnerConeAngle(BaseOuterAngle * TargetData->InnerConeRatio);
				FlashLightSpot->SetCastShadows(TargetData->bCastShadows);
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

	if (Player->GetCurrentWeaponIndex() == 0) 
	{
		if (bIsUseFlashlight && RaiseLightMontage)
			Player->PlayAnimMontage(RaiseLightMontage);
		else if (!bIsUseFlashlight && LowerLightMontage)
			Player->PlayAnimMontage(LowerLightMontage);
	}
	UpdateFlashlight(0.0f);
}