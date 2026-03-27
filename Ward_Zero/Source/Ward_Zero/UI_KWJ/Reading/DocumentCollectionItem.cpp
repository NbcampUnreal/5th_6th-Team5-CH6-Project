// DocumentCollectionItem.cpp

#include "UI_KWJ/Reading/DocumentCollectionItem.h"
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

void UDocumentCollectionItem::SetDocumentInfoFromEntry(const FWardDocumentEntry& Entry, bool bUnlocked)
{
	CachedDocIndex = Entry.DocIndex;
	bIsUnlocked = bUnlocked;

	// 제목
	if (TXT_Title)
	{
		TXT_Title->SetText(bUnlocked ? Entry.Title : FText::FromString(TEXT("???")));
	}

	// 페이지 수
	if (TXT_PageCount)
	{
		TXT_PageCount->SetVisibility(bUnlocked ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// 썸네일
	if (IMG_Thumbnail)
	{
		if (bUnlocked && Entry.ThumbnailImage.IsValid())
		{
			UTexture2D* Tex = Entry.ThumbnailImage.LoadSynchronous();
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
			IMG_Thumbnail->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.f));
		}
	}

	// 잠금 아이콘
	if (IMG_Lock)
	{
		IMG_Lock->SetVisibility(bUnlocked ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	// 버튼
	if (BTN_Item)
	{
		BTN_Item->SetIsEnabled(bUnlocked);
	}
}

void UDocumentCollectionItem::OnItemClicked()
{
	if (bIsUnlocked && CachedDocIndex >= 0)
	{
		OnClicked_Index.ExecuteIfBound(CachedDocIndex);
	}
}
