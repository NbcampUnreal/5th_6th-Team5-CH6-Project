// WardDocumentDataTable.h
// 모든 문서/아이템 설명을 하나의 테이블에서 관리
// 인덱스로 접근 — 액터는 인덱스만 가지면 됨
//
// 인덱스 규칙:
//   0~19: 아이템 설명 (힐아이템=0, 총알박스=1, ...)
//   20~: 서류 문서

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WardDocumentDataTable.generated.h"

/** 문서/아이템 한 항목의 데이터 */
USTRUCT(BlueprintType)
struct FWardDocumentEntry
{
	GENERATED_BODY()

	/** 고유 인덱스 (액터가 참조하는 값) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	int32 DocIndex = -1;

	/** 제목/이름 (UI 표시용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	FText Title;

	/** 설명 텍스트 (아이템 알림 / 서류 뷰어에서 표시) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	FText Description;

	/** 수집 UI 썸네일 (메모 아이콘, 카드보드 아이콘 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	TSoftObjectPtr<UTexture2D> ThumbnailImage;

	/** 서류 뷰어 뒷배경 (편지지, 노트 텍스처 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	TSoftObjectPtr<UTexture2D> BackgroundImage;

	/** 사용 키 안내 (아이템 전용, 예: "Q키를 눌러 사용") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	FText KeyHint;

	/** 서류 페이지 목록 (서류 전용 — 아이템이면 비어있음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document", meta = (MultiLine = true))
	TArray<FText> Pages;
};

/** 모든 문서/아이템 데이터를 담는 DataAsset */
UCLASS(BlueprintType)
class WARD_ZERO_API UWardDocumentDataTable : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** 전체 항목 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	TArray<FWardDocumentEntry> Entries;

	/** 인덱스로 항목 찾기 (없으면 nullptr) */
	const FWardDocumentEntry* FindByIndex(int32 DocIndex) const
	{
		for (const FWardDocumentEntry& Entry : Entries)
		{
			if (Entry.DocIndex == DocIndex)
			{
				return &Entry;
			}
		}
		return nullptr;
	}
};
