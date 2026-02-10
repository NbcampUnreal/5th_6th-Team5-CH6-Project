// DocumentSubsystem.h
// LocalPlayer 서브시스템 - 서류 뷰어를 중앙에서 관리

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "DocumentSubsystem.generated.h"

class UDocumentData;
class UDocumentViewerWidget;

UCLASS()
class UDocumentSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 서류 열기 */
	UFUNCTION(BlueprintCallable, Category = "Document")
	void OpenDocument(UDocumentData* InDocument);

	/** 서류 닫기 */
	UFUNCTION(BlueprintCallable, Category = "Document")
	void CloseDocument();

	/** 현재 서류가 열려있는지 */
	UFUNCTION(BlueprintPure, Category = "Document")
	bool IsDocumentOpen() const;

	/** 다음 페이지 */
	UFUNCTION(BlueprintCallable, Category = "Document")
	void TurnPageNext();

	/** 이전 페이지 */
	UFUNCTION(BlueprintCallable, Category = "Document")
	void TurnPagePrev();

	/** 상호작용 힌트 UI 표시 */
	UFUNCTION(BlueprintCallable, Category = "Document")
	void ShowInteractionHint(const FText& HintText);

	/** 상호작용 힌트 UI 숨기기 */
	UFUNCTION(BlueprintCallable, Category = "Document")
	void HideInteractionHint();

protected:

	UPROPERTY()
	TSubclassOf<UDocumentViewerWidget> ViewerWidgetClass;

	UPROPERTY()
	UDocumentViewerWidget* ViewerWidget;

	UDocumentViewerWidget* GetOrCreateViewer();
};
