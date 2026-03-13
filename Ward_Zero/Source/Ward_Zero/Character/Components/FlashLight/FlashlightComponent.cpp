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

	// [수정] 모든 부착된 무기(등에 멘 무기 포함)의 라이트와 발광을 매 틱 초기화합니다.
	// 이 과정이 없으면 무기 교체 시 이전에 켜졌던 SMG의 빛이 꺼지지 않고 남습니다.
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

	// [발광 제어값] 켰을 때 50, 껐을 때 0
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
		// [원본 유지] 달릴 때는 가슴/손 손전등을 사용하므로 SMG의 SpotLight는 끔 (위의 루프에서 이미 처리됨)

		// Unarmed -> 왼손 소켓 / Weapon -> 가슴 소켓 
		FName RunSocket = bIsUnarmed ? TEXT("FlashLightSocket_Normal") : TEXT("BodyLightSocket");
		FlashLightActor->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RunSocket);

		// 액터 자체는 켜두어야 빛이 나옴, 대신 메쉬(모델링)만 투명하게 숨김
		FlashLightActor->SetActorHiddenInGame(false);

		// Unarmed 일때는 손전등 보여줌 
		if (FlashLightMesh)
		{
			FlashLightMesh->SetVisibility(bIsUnarmed);
			// [수정] 손전등 본체 발광 제어
			FlashLightMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), bIsUnarmed ? TargetEmissive : 0.0f);
		}

		// [추가] 달리기 중에도 현재 든 무기가 SMG라면 발광(Emissive)은 유지해줌
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

			// [추가] SMG 발광 On
			if (UMeshComponent* WeaponMesh = CurrentWeapon->FindComponentByClass<UMeshComponent>())
				WeaponMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), TargetEmissive);

			ActiveSpot = CurrentWeapon->SMGSpotLight;
		}
		// 권총이거나 비무장일 때
		else
		{
			// [수정] SMG 라이트가 아닐 때 SMG 라이트를 끄는 로직은 상단 루프에서 처리됨

			bool bShouldHide = (AnimIF->GetIsReloading() || AnimIF->IsEquipping()) && !bIsUnarmed;
			if (bIsUnarmed) bShouldHide = false;

			FName SocketName = AnimIF->GetIsPistolEquipped() ? TEXT("FlashLightSocket_Pistol") : TEXT("FlashLightSocket_Normal");

			// 손 소켓으로 이동
			FlashLightActor->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
			FlashLightActor->SetActorRelativeLocation(FVector::ZeroVector);
			FlashLightActor->SetActorRelativeRotation(FRotator::ZeroRotator);

			FlashLightActor->SetActorHiddenInGame(bShouldHide);

			// 메쉬 다시 보이게 복구
			if (FlashLightMesh)
			{
				FlashLightMesh->SetVisibility(!bShouldHide);
				// [수정] 손전등 본체 발광 제어
				FlashLightMesh->SetScalarParameterValueOnMaterials(TEXT("Intensity"), bShouldHide ? 0.0f : TargetEmissive);
			}

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