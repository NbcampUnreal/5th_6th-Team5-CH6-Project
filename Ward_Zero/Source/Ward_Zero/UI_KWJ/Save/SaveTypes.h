// Fill out your copyright notice in the Description page of Project Settings.

// SaveTypes.h
// FSaveFileInfo 구조체 순환 참조 방지를 위해 별도 헤더로 분리

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "SaveTypes.generated.h"

USTRUCT(BlueprintType)
struct FSaveFileInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString SlotName;

	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly)
	FDateTime SaveDateTime;

	UPROPERTY(BlueprintReadOnly)
	float PlayTimeSeconds = 0.f;

	UPROPERTY(BlueprintReadOnly)
	FName LevelName;

	UPROPERTY(BlueprintReadOnly)
	UTexture2D* Thumbnail = nullptr;
};
