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

	if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
	{
		// 1. 가슴 라이트(BodyRunLight) 동적 생성 및 설정
		BodyRunLight = NewObject<USpotLightComponent>(Owner, TEXT("BodyRunLight"));
		if (BodyRunLight)
		{
			BodyRunLight->RegisterComponent();
			BodyRunLight->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			BodyRunLight->SetRelativeLocation(FVector(30.f, 0.f, 30.f));
			BodyRunLight->SetVisibility(false);
			BodyRunLight->SetCastShadows(false);
		}

		// 2. [추가된 부분] 실제 손전등 액터(FlashLightActor) 월드에 스폰
		if (FlashLightClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Owner;
			SpawnParams.Instigator = Owner; // 스폰 주체 명시

			FlashLightActor = GetWorld()->SpawnActor<AFlashLight>(FlashLightClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

			if (FlashLightActor)
			{
				FlashLightActor->SetActorEnableCollision(false);
				FlashLightActor->SetActorHiddenInGame(true); // 처음엔 꺼진 상태로 대기
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("FlashLightComponent: FlashLightClass가 블루프린트에 할당되지 않았습니다!"));
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
	APrototypeCharacter* Player = Cast<APrototypeCharacter>(GetOwner());
	if (!Player) return;

	// 인터페이스를 통해 필요한 상태값들을 가져옵니다.
	IPlayerAnimInterface* AnimIF = Cast<IPlayerAnimInterface>(Player);
	if (!AnimIF) return;

	UPlayerCombatComponent* CombatComp = Player->FindComponentByClass<UPlayerCombatComponent>();
	if (!CombatComp) return;

	// 초기화 -> 모든 무기 라이트 상태 초기화
	AWeapon* CurrentWeapon = Player->GetEquippedWeapon();
	if (CombatComp)
	{
		TArray<AWeapon*> AllWeapons;
		if (CombatComp->PistolWeapon) AllWeapons.Add(CombatComp->PistolWeapon);
		if (CombatComp->SMGWeapon) AllWeapons.Add(CombatComp->SMGWeapon);

		for (AWeapon* Weapon : AllWeapons)
		{
			if (Weapon && Weapon->SMGSpotLight)
			{
				Weapon->SMGSpotLight->SetVisibility(false);

				// 메쉬 발광(Emissive)도 초기화
				if (Weapon->SMGLight)
				{
					UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(Weapon->SMGLight->GetMaterial(0));
					if (DynMat) DynMat->SetScalarParameterValue(TEXT("Intensity"), 0.0f);
				}
			}
		}
	}

	if (FlashLightActor)
	{
		FlashLightActor->SetActorHiddenInGame(true);
		if (auto* Spot = FlashLightActor->FindComponentByClass<USpotLightComponent>()) Spot->SetVisibility(false);
	}
	if (BodyRunLight) BodyRunLight->SetVisibility(false);

	if (!bIsUseFlashlight) return;

	if (CurrentWeapon && CurrentWeapon->SMGSpotLight) {
		CurrentWeapon->SMGSpotLight->SetVisibility(false);
		CurrentWeapon->SMGSpotLight->SetCastShadows(false); // 그림자 간섭 방지
	}

	// 데이터 에셋 설정: 무기 전용 설정이 있으면 쓰고, 없으면 기본값(DefaultFlashlightData) 사용
	UFlashLightData* TargetData = (CurrentWeapon && CurrentWeapon->WeaponData && CurrentWeapon->WeaponData->FlashlightSettings)
		? CurrentWeapon->WeaponData->FlashlightSettings : DefaultFlashlightData;

	if (!TargetData) return;

	// SMG 라이트 초기화
	if (CurrentWeapon && CurrentWeapon->SMGSpotLight) {
		CurrentWeapon->SMGSpotLight->SetVisibility(false);

		// SMG 메쉬 발광(Emissive) 재질 처리
		if (CurrentWeapon->SMGLight) {
			UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(CurrentWeapon->SMGLight->GetMaterial(0));
			if (DynMat) DynMat->SetScalarParameterValue(TEXT("Intensity"), 0.0f);
		}
	}

	if (!bIsUseFlashlight) return;

	// SMG 장착
	if (AnimIF->GetIsSMGEquipped() && CurrentWeapon)
	{
		if (AnimIF->GetIsRunning() && BodyRunLight)
		{
			// SMG 달리기: 총구 라이트는 끄고 가슴(Body) 라이트 활성화
			BodyRunLight->SetIntensity(TargetData->Intensity * 0.3f);
			BodyRunLight->SetOuterConeAngle(TargetData->OuterConeAngle * 0.8f);
			BodyRunLight->SetAttenuationRadius(TargetData->AttenuationRadius * 0.4f);
			BodyRunLight->SetVisibility(true);
			BodyRunLight->SetCastShadows(false);
		}
		else
		{
			// SMG 평상시: 총구 소켓 라이트 및 메쉬 발광 활성화
			if (CurrentWeapon->SMGSpotLight) {
				CurrentWeapon->SMGSpotLight->SetIntensity(TargetData->Intensity);
				CurrentWeapon->SMGSpotLight->SetOuterConeAngle(TargetData->OuterConeAngle);
				CurrentWeapon->SMGSpotLight->SetAttenuationRadius(TargetData->AttenuationRadius);
				CurrentWeapon->SMGSpotLight->SetVisibility(true);
				CurrentWeapon->SMGSpotLight->SetCastShadows(false);
			}
			if (CurrentWeapon->SMGLight) {
				UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(CurrentWeapon->SMGLight->GetMaterial(0));
				if (DynMat) DynMat->SetScalarParameterValue(TEXT("Intensity"), TargetData->EmissiveIntensity);
			}
		}
	}
	// Pistol / Unarmed
	else
	{
		if (FlashLightActor)
		{
			// 장전 중이거나 무기 교체 중이면 액터 숨김 처리
			bool bShouldHide = AnimIF->GetIsReloading() || AnimIF->IsEquipping();

			if (AnimIF->GetIsRunning() && BodyRunLight)
			{
				// Pistol/Unarmed: 손전등 액터는 숨기고 가슴 라이트 활성화
				FlashLightActor->SetActorHiddenInGame(false);

				if (auto* Spot = FlashLightActor->FindComponentByClass<USpotLightComponent>()) {
					Spot->SetVisibility(false);
				}

				BodyRunLight->SetIntensity(TargetData->Intensity * 0.7f);
				BodyRunLight->SetOuterConeAngle(TargetData->OuterConeAngle * 0.8f);
				BodyRunLight->SetAttenuationRadius(TargetData->AttenuationRadius * 0.6f);
				BodyRunLight->SetVisibility(true);
			}
			else
			{
				// 권총/맨손 평상시: 손전등 액터를 소켓에 부착하고 위치 초기화

				// 1) 소켓 이름 결정 (원본 로직: 권총 유무에 따라 소켓 변경)
				FName SocketName = AnimIF->GetIsPistolEquipped() ? TEXT("FlashLightSocket_Pistol") : TEXT("FlashLightSocket_Normal");

				// 2) 소켓에 부착 (달리기 종료 후 복귀 시 위치 갱신 필수)
				FlashLightActor->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

				// 3) [중요] 위치와 회전을 소켓 원점으로 강제 고정
				FlashLightActor->SetActorRelativeLocation(FVector::ZeroVector);
				FlashLightActor->SetActorRelativeRotation(FRotator::ZeroRotator);

				// 장전/장착 중이 아닐 때만 라이트를 켬
				if (!bShouldHide)
				{
					FlashLightActor->SetActorHiddenInGame(false);
					FlashLightActor->InitializeLight(TargetData);

					if (auto* Spot = FlashLightActor->FindComponentByClass<USpotLightComponent>()) {
						Spot->SetVisibility(true);
					}
				}

				// 가슴 라이트는 끔
				if (BodyRunLight) BodyRunLight->SetVisibility(false);
			}
		}
	}
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