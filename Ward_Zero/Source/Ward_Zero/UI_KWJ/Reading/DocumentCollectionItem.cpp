// DocumentCollectionItem.cpp

#include "UI_KWJ/Reading/DocumentCollectionItem.h"
#include "UI_KWJ/Reading/DocumentData.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"
#include "Ward_Zero.h"

void UDocumentCollectionItem::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (BTN_Item)
	{
		BTN_Item->OnClicked.AddDynamic(this, &UDocumentCollectionItem::OnItemClicked);
	}
}

void UDocumentCollectionItem::SetDocumentInfo(UDocumentData* InDocument, bool bUnlocked)
{
	CachedDocument = InDocument;
	bIsUnlocked = bUnlocked;

	if (!InDocument)
	{
		// 빈 슬롯
		if (TXT_Title) TXT_Title->SetText(FText::FromString(TEXT("???")));
		if (TXT_PageCount) TXT_PageCount->SetVisibility(ESlateVisibility::Collapsed);
		if (IMG_Lock) IMG_Lock->SetVisibility(ESlateVisibility::Visible);
		if (BTN_Item) BTN_Item->SetIsEnabled(false);
		return;
	}

	// 제목
	if (TXT_Title)
	{
		if (bUnlocked)
		{
			TXT_Title->SetText(InDocument->DocumentTitle);
		}
		else
		{
			TXT_Title->SetText(FText::FromString(TEXT("???")));
		}
	}

	// 페이지 수
	if (TXT_PageCount)
	{
		if (bUnlocked)
		{
			FString PageStr = FString::Printf(TEXT("Pages: %d"), InDocument->Pages.Num());
			TXT_PageCount->SetText(FText::FromString(PageStr));
			TXT_PageCount->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			TXT_PageCount->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 썸네일 (배경 텍스처 사용)
	if (IMG_Thumbnail)
	{
		if (bUnlocked && InDocument->BackgroundTexture.IsValid())
		{
			UTexture2D* Tex = InDocument->BackgroundTexture.Get();
			if (!Tex)
			{
				Tex = InDocument->BackgroundTexture.LoadSynchronous();
			}

			if (Tex)
			{
				FSlateBrush Brush;
				Brush.SetResourceObject(Tex);
				Brush.ImageSize = FVector2D(200.f, 260.f);
				Brush.DrawAs = ESlateBrushDrawType::Image;
				IMG_Thumbnail->SetBrush(Brush);
				IMG_Thumbnail->SetColorAndOpacity(FLinearColor::White);
			}
		}
		else
		{
			// 잠금 상태 — 어둡게
			IMG_Thumbnail->SetColorAndOpacity(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f));
		}
	}

	// 잠금 아이콘
	if (IMG_Lock)
	{
		IMG_Lock->SetVisibility(bUnlocked ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	// 버튼 활성화
	if (BTN_Item)
	{
		BTN_Item->SetIsEnabled(bUnlocked);
	}
}

void UDocumentCollectionItem::OnItemClicked()
{
	if (bIsUnlocked && CachedDocument)
	{
		OnClicked_Item.ExecuteIfBound(CachedDocument);
	}
}
