// DocumentData.h
// 서류 데이터 정의 - DataAsset 기반

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DocumentData.generated.h"

/**
 *  서류 한 페이지의 데이터
 */
USTRUCT(BlueprintType)
struct FDocumentPageData
{
	GENERATED_BODY()

	/** 페이지 본문 텍스트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true))
	FText PageText;
};

/**
 *  서류 한 장의 전체 데이터 (DataAsset)
 *  에디터에서 Content Browser > 우클릭 > Miscellaneous > Data Asset > DocumentData 로 생성
 */
UCLASS(BlueprintType)
class UDocumentData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	/** 서류 제목 (인벤토리 등에서 표시용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	FText DocumentTitle;

	/** 페이지 목록 - 페이지가 1개면 Turn Page 버튼 숨김 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	TArray<FDocumentPageData> Pages;

	/** 서류 배경 텍스처 (책, 노트, 편지지 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document")
	TSoftObjectPtr<UTexture2D> BackgroundTexture;
};
