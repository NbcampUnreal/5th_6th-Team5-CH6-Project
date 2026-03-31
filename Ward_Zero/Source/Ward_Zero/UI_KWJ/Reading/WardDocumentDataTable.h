// WardDocumentDataTable.h
// 모든 문서/아이템 설명을 하나의 테이블에서 관리
// 인덱스로 접근 — 액터는 인덱스만 가지면 됨
//
// 인덱스 규칙:
//   0~19: 아이템 설명 (힐아이템=0, 총알박스=1, ...)
//   20~: 서류 문서
//
// 서류 텍스트 파일 규칙:
//   Content/Documents/Doc_20.txt ← DocIndex 20이면 자동으로 이 파일
//   파일 안에서 --- 로 페이지 구분

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
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

	/** 설명 텍스트 (아이템 알림용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	FText Description;

	/** 수집 UI 썸네일 (메모 아이콘, 카드보드 아이콘 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	TSoftObjectPtr<UTexture2D> ThumbnailImage;

	/** 서류 뷰어 뒷배경 (편지지, 노트 텍스처 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	TSoftObjectPtr<UTexture2D> BackgroundImage;

	/** 아이템 알림 표시명 (노티파이에 뜨는 이름, 예: "서류", "치료제")
	 *  비어있으면 Title을 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	FText NotifyDisplayName;

	/** 사용 키 안내 (아이템 전용, 예: "Q키를 눌러 사용") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	FText KeyHint;

	/** 서류 페이지 목록 (런타임에 txt 파일에서 자동 로드) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Document")
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

	/** 인덱스로 항목 찾기 (Pages가 비어있으면 txt에서 자동 로드) */
	const FWardDocumentEntry* FindByIndex(int32 DocIndex)
	{
		for (FWardDocumentEntry& Entry : Entries)
		{
			if (Entry.DocIndex == DocIndex)
			{
				// Pages가 비어있으면 Content/Documents/Doc_{인덱스}.txt에서 자동 로드
				if (Entry.Pages.Num() == 0)
				{
					LoadPagesFromFile(Entry);
				}
				return &Entry;
			}
		}
		return nullptr;
	}

	/** txt 파일에서 페이지 로드 — Content/Documents/Doc_{DocIndex}.txt
	 *  --- 구분자로 페이지 분리 */
	static void LoadPagesFromFile(FWardDocumentEntry& Entry)
	{
		// Content/Documents/Doc_20.txt 형식
		FString FileName = FString::Printf(TEXT("Doc_%d.txt"), Entry.DocIndex);
		FString FilePath = FPaths::ProjectContentDir() / TEXT("Documents") / FileName;
		FString FileContent;

		if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
		{
			UE_LOG(LogTemp, Warning, TEXT("서류 파일 로드 실패: %s"), *FilePath);
			return;
		}

		// --- 로 페이지 분리
		TArray<FString> PageStrings;
		FileContent.ParseIntoArray(PageStrings, TEXT("---"), true);

		for (const FString& PageStr : PageStrings)
		{
			FString Trimmed = PageStr.TrimStartAndEnd();
			if (!Trimmed.IsEmpty())
			{
				Entry.Pages.Add(FText::FromString(Trimmed));
			}
		}

		UE_LOG(LogTemp, Log, TEXT("서류 파일 로드: %s (%d 페이지)"), *FileName, Entry.Pages.Num());
	}
};
