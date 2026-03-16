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

	TArray<AActor*> AttachedActors;
	Player->GetAttachedActors(AttachedActors);
	for (AActor* Actor : AttachedActors)
	{
		if (AWeapon* WeaponActor = Cast<AWeapon>(Actor))
		{
			// SMG 라이트 끄기
			if (WeaponActor->SMGSpotLight)
			{
				WeaponActor->SMGSpotLight->SetVisibility(false);
			}

			// 무기 머티리얼 발광(Intensity) 끄기
			if (UMeshComponent* WMesh = WeaponActor->FindComponentByClass<UMeshComponent>())
			{
				WMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), 0.0f);
			}
		}
	}

	// 발광 제어값 => 켰을 때 50, 껐을 때 0
	float TargetEmissive = bIsUseFlashlight ? 50.0f : 0.0f;

	// 전등 OFF 상태 처리
	if (!bIsUseFlashlight)
	{
		FlashLightActor->SetActorHiddenInGame(true);
		FlashLightSpot->SetVisibility(false);

		// 손전등 본체 머티리얼 발광 끄기
		if (FlashLightMesh) FlashLightMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), 0.0f);

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
		// 달릴 때는 가슴/손 손전등을 사용하므로 SMG의 SpotLight는 끔 

		// Unarmed -> 왼손 소켓 / Weapon -> 가슴 소켓 
		FName RunSocket = bIsUnarmed ? TEXT("FlashLightSocket_Normal") : TEXT("BodyLightSocket");
		FlashLightActor->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RunSocket);

		// 액터 자체는 켜두어야 빛이 나옴, 대신 메쉬(모델링)만 투명하게 숨김
		FlashLightActor->SetActorHiddenInGame(false);

		// Unarmed 일때는 손전등 보여줌 
		if (FlashLightMesh)
		{
			FlashLightMesh->SetVisibility(bIsUnarmed);
			// 손전등 본체 발광 제어
			FlashLightMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), bIsUnarmed ? TargetEmissive : 0.0f);
		}

		// 달리기 중에도 현재 든 무기가 SMG라면 발광(Emissive)은 유지해줌
		if (AnimIF->GetIsSMGEquipped() && CurrentWeapon)
		{
			if (UMeshComponent* WeaponMesh = CurrentWeapon->FindComponentByClass<UMeshComponent>())
				WeaponMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), TargetEmissive);
		}

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
			if (FlashLightMesh) FlashLightMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), 0.0f);

			// SMG 라이트 흔들림 무시 + 카메라 정면으로 위치 고정 
			CurrentWeapon->SMGSpotLight->SetUsingAbsoluteRotation(true);
			CurrentWeapon->SMGSpotLight->SetWorldRotation(Player->GetControlRotation());
			CurrentWeapon->SMGSpotLight->SetVisibility(true);

			CurrentWeapon->SMGSpotLight->SetIntensity(BaseIntensity);
			CurrentWeapon->SMGSpotLight->SetOuterConeAngle(BaseOuterAngle);
			CurrentWeapon->SMGSpotLight->SetInnerConeAngle(BaseOuterAngle * TargetData->InnerConeRatio);
			CurrentWeapon->SMGSpotLight->SetAttenuationRadius(TargetData->AttenuationRadius);
			CurrentWeapon->SMGSpotLight->SetCastShadows(TargetData->bCastShadows);

			// SMG 발광 On
			if (UMeshComponent* WeaponMesh = CurrentWeapon->FindComponentByClass<UMeshComponent>())
				WeaponMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), TargetEmissive);

			ActiveSpot = CurrentWeapon->SMGSpotLight;
		}
		// 권총이거나 비무장일 때
		else
		{
			bool bIsPistol = AnimIF->GetIsPistolEquipped();
			bool bIsAiming = AnimIF->GetIsAiming();
			bool IsUnarmed = !CachedCombatComp->IsWeaponDrawn();

			bool bShouldHide = (AnimIF->GetIsReloading() || AnimIF->IsEquipping()) && !IsUnarmed;
			if (IsUnarmed) bShouldHide = false;

			FName SocketName;

			if (bIsPistol && !bIsAiming)
			{
				// 어깨 소켓에 부착
				SocketName = TEXT("BodyLightSocket");

				// 어깨에 달 때는 손전등 모델(Mesh)을 숨깁니다 (몸 안에 파묻히는 것 방지)
				if (FlashLightMesh) FlashLightMesh->SetVisibility(false);
			}
			else if (bIsPistol && bIsAiming)
			{
				// 조준 중일 때는 기존처럼 권총 손 소켓
				SocketName = TEXT("FlashLightSocket_Pistol");
				if (FlashLightMesh) FlashLightMesh->SetVisibility(!bShouldHide);
			}
			else
			{
				// 비무장(Unarmed)일 때
				SocketName = TEXT("FlashLightSocket_Normal");
				if (FlashLightMesh) FlashLightMesh->SetVisibility(!bShouldHide);
			}

			FlashLightActor->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
			FlashLightActor->SetActorRelativeLocation(FVector::ZeroVector);
			FlashLightActor->SetActorRelativeRotation(FRotator::ZeroRotator);

			FlashLightActor->SetActorHiddenInGame(false); // 라이트는 나와야 하므로 액터는 켬

			// 빛 설정: 조준 시(Base) 설정을 그대로 적용
			FlashLightSpot->SetUsingAbsoluteRotation(true);
			FlashLightSpot->SetWorldRotation(Player->GetControlRotation());
			FlashLightSpot->SetVisibility(!bShouldHide);

			if (!bShouldHide)
			{
				// 어깨에 달았을 때도 TargetData의 기본 Intensity와 Angle을 그대로 사용 (조준 시와 동일)
				FlashLightSpot->SetIntensity(BaseIntensity);
				FlashLightSpot->SetOuterConeAngle(BaseOuterAngle);
				FlashLightSpot->SetInnerConeAngle(BaseOuterAngle * TargetData->InnerConeRatio);
				FlashLightSpot->SetAttenuationRadius(TargetData->AttenuationRadius);
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
	// 트레이스 시작점을 총구에서 조금 더 앞으로 밀어 충돌 방지
	FVector Start = Light->GetComponentLocation() + (Light->GetForwardVector() * 15.0f);
	FVector End = Start + (Light->GetForwardVector() * MaxDist);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	if (FlashLightActor) Params.AddIgnoredActor(FlashLightActor);

	// 무기 메쉬 무시 (깜빡임 방지 핵심)
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