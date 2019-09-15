#include "GoogleDoc2DataTable.h"
#include "LevelEditor.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"

#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
#include "Runtime/Engine/Classes/Engine/DataTable.h"
#include "Runtime/Engine/Classes/Kismet/DataTableFunctionLibrary.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"

#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

void FGoogleDoc2DataTableModule::StartupModule()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBAssetMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FGoogleDoc2DataTableModule::DataTableContextMenuExtender));
	CBAssetExtenderDelegateHandle = CBAssetMenuExtenderDelegates.Last().GetHandle();
}

void FGoogleDoc2DataTableModule::ShutdownModule()
{
	if (!IsRunningCommandlet() && !IsRunningGame())
	{
		FContentBrowserModule* ContentBrowserModule = (FContentBrowserModule*)(FModuleManager::Get().GetModule(TEXT("ContentBrowser")));
		if (ContentBrowserModule)
		{
			TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
			CBAssetMenuExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == CBAssetExtenderDelegateHandle; });
		}
	}
}

TSharedRef<FExtender> FGoogleDoc2DataTableModule::DataTableContextMenuExtender(const TArray<FAssetData>& AssetDataList)
{
	if (AssetDataList.Num() != 1 || AssetDataList[0].AssetClass != UDataTable::StaticClass()->GetFName())
		return MakeShareable(new FExtender());

	SelectedDataTable = Cast<UDataTable>(AssetDataList[0].GetAsset());

	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());

	MenuExtender->AddMenuExtension(
		"ImportedAssetActions",
		EExtensionHook::Before,
		TSharedPtr<FUICommandList>(),
		FMenuExtensionDelegate::CreateRaw(this, &FGoogleDoc2DataTableModule::AddMenuEntry));

	return MenuExtender.ToSharedRef();
}

void FGoogleDoc2DataTableModule::AddMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString("Load from Google doc"),
		FText::FromString("Load from Google doc"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FGoogleDoc2DataTableModule::ShowWindow))
	);
}

void FGoogleDoc2DataTableModule::ShowWindow()
{
	TSharedRef<SWindow> Window =
		SNew(SWindow).Title(FText::FromString(TEXT("Load data from Google Spreadsheet")))
		.ClientSize(FVector2D(800, 100))
		.SupportsMaximize(false)
		.SupportsMinimize(false)[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Spreadsheet id:")))
						]
					+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							SNew(SEditableTextBox)
							.MinDesiredWidth(800)
						.Text(LastDocId)
						.BackgroundColor(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)))
						.OnTextChanged_Raw(this, &FGoogleDoc2DataTableModule::TextChanged)
						]
				]
				+ SVerticalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[
					SNew(SButton)
						.Text(FText::FromString(TEXT("Ok")))
						.OnClicked_Raw(this, &FGoogleDoc2DataTableModule::LoadGoogleDoc)
				]
		];

	MainWindow = Window;
	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));

	if (MainFrameModule.GetParentWindow().IsValid()) {
		FSlateApplication::Get().AddWindowAsNativeChild(Window, MainFrameModule.GetParentWindow().ToSharedRef());
	}
	else {
		FSlateApplication::Get().AddWindow(Window);
	}
}

FReply FGoogleDoc2DataTableModule::LoadGoogleDoc() 
{
	GoogleDocsApi = NewObject<UGoogleDocsApi>(UGoogleDocsApi::StaticClass());
	GoogleDocsApi->OnResponseDelegate.BindRaw(this, &FGoogleDoc2DataTableModule::OnApiResponse);
	GoogleDocsApi->SendRequest(LastDocId.ToString());

	if (MainWindow.IsValid())
	{
		MainWindow.Pin()->RequestDestroyWindow();
	}

	return FReply::Handled();
}

void FGoogleDoc2DataTableModule::TextChanged(const FText& DocId)
{
	LastDocId = DocId;
}

void FGoogleDoc2DataTableModule::OnApiResponse(FString Response) 
{
	UDataTableFunctionLibrary::FillDataTableFromCSVString(SelectedDataTable, Response);
}

IMPLEMENT_MODULE(FGoogleDoc2DataTableModule, GoogleDoc2DataTable)