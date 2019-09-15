#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Runtime/Engine/Classes/Engine/DataTable.h"

#include "GoogleDocsApi.h"

class FGoogleDoc2DataTableModule : public IModuleInterface
{

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TWeakPtr<SWindow> MainWindow;
	UDataTable* SelectedDataTable;
	UGoogleDocsApi* GoogleDocsApi;
	FDelegateHandle CBAssetExtenderDelegateHandle;
	FText LastDocId;

	void ShowWindow();
	void AddMenuEntry(FMenuBuilder& MenuBuilder);
	void TextChanged(const FText& DocId);
	void OnApiResponse(FString Response);

	TSharedRef<FExtender> DataTableContextMenuExtender(const TArray<FAssetData>& AssetDataList);
	FReply LoadGoogleDoc();
};
