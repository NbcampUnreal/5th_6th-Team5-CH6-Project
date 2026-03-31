// DocumentSubsystem.h
// LocalPlayer 서브시스템 - 서류 뷰어를 중앙에서 관리

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "DocumentSubsystem.generated.h"

class UDocumentData;
class UDocumentViewerWidget;
class UDocumentCollectionWidget;

UCLASS()
class UDocumentSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ══════════════════════════════════════════
	//  서류 열기/닫기
	// ══════════════════════════════════════════

	UFUNCTION(BlueprintCallable, Category = "Document")
	void OpenDocument(UDocumentData* InDocument);

	/** 서류 열기 (인덱스로 DataTable에서 조회 → 뷰어 표시) */
	UFUNCTION(BlueprintCallable, Category = "Document")
	void OpenDocumentByIndex(int32 DocIndex, bool bImmediate = false);

	UFUNCTION(BlueprintCallable, Category = "Document")
	void CloseDocument();

	UFUNCTION(BlueprintPure, Category = "Document")
	bool IsDocumentOpen() const;

	UFUNCTION(BlueprintCallable, Category = "Document")
	void TurnPageNext();

	UFUNCTION(BlueprintCallable, Category = "Document")
	void TurnPagePrev();

	UFUNCTION(BlueprintCallable, Category = "Document")
	void ShowInteractionHint(const FText& HintText);

	UFUNCTION(BlueprintCallable, Category = "Document")
	void HideInteractionHint();

	// ══════════════════════════════════════════
	//  서류 수집 관리
	// ══════════════════════════════════════════

	/** 서류 수집 등록 (서류 줍기 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "Document|Collection")
	void CollectDocument(UDocumentData* InDocument);

	/** 서류가 수집되었는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Document|Collection")
	bool IsDocumentCollected(UDocumentData* InDocument) const;

	/** 전체 서류 목록 가져오기 */
	const TArray<UDocumentData*>& GetAllDocuments() const { return AllDocuments; }

	/** 수집된 서류 세트 가져오기 */
	const TSet<UDocumentData*>& GetCollectedDocuments() const { return CollectedDocuments; }

	// ══════════════════════════════════════════
	//  컬렉션 UI
	// ══════════════════════════════════════════

	/** 서류 수집 UI 표시 (ESC 메뉴에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Document|Collection")
	void ShowCollection();

	/** 서류 수집 UI 숨기기 */
	UFUNCTION(BlueprintCallable, Category = "Document|Collection")
	void HideCollection();

	/** 서류 수집 UI가 열려있는지 */
	UFUNCTION(BlueprintPure, Category = "Document|Collection")
	bool IsCollectionOpen() const;
	/** 수집 UI에서 서류를 열었는지 여부 */
	bool bOpenedFromCollection = false;

	/** 닫기 애니메이션 재생 중 (ESC 중복 입력 방지) */
	bool bIsClosing = false;

protected:

	UPROPERTY()
	TSubclassOf<UDocumentViewerWidget> ViewerWidgetClass;

	UPROPERTY()
	UDocumentViewerWidget* ViewerWidget;

	UDocumentViewerWidget* GetOrCreateViewer();

	// ── 수집 데이터 ──

	/** 게임 내 전체 서류 목록 (에디터에서 등록) */
	UPROPERTY(EditDefaultsOnly, Category = "Document|Collection")
	TArray<UDocumentData*> AllDocuments;

	/** 수집한 서류 세트 */
	UPROPERTY()
	TSet<UDocumentData*> CollectedDocuments;

	// ── 컬렉션 위젯 ──

	UPROPERTY()
	TSubclassOf<UDocumentCollectionWidget> CollectionWidgetClass;

	UPROPERTY()
	UDocumentCollectionWidget* CollectionWidget;

	UDocumentCollectionWidget* GetOrCreateCollection();

	// ── 노티파이 대기 후 서류 열기 ──

	/** 노티파이 닫힘 후 열어야 할 서류 인덱스 (-1이면 없음) */
	int32 PendingDocIndex = -1;

	/** 한 틱 지연 후 서류 열기 (노티파이와 동시 호출 대응) */
	void OpenDocumentByIndexDeferred();

	/** 노티파이 닫힌 후 대기 중이던 서류 열기 */
	void OnNotifyHiddenOpenPending();
};
