#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Http.h"
#include "GoogleDocsApi.generated.h"

DECLARE_DELEGATE_OneParam(OnResponse, FString);

UCLASS()
class GOOGLEDOC2DATATABLE_API UGoogleDocsApi : public UObject
{
	GENERATED_BODY()

public:	
	OnResponse OnResponseDelegate;

	void SendRequest(FString docId);

private:
	static const FString ApiBaseUrl;
	FHttpModule* Http;

	void ProcessResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SetRequestHeaders(TSharedRef<IHttpRequest,ESPMode::ThreadSafe>& Request);
	bool ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful);

	TSharedRef<IHttpRequest,ESPMode::ThreadSafe> GetRequest(FString Subroute);
	TSharedRef<IHttpRequest,ESPMode::ThreadSafe> PostRequest(FString Subroute, FString ContentJsonString);
	TSharedRef<IHttpRequest,ESPMode::ThreadSafe> RequestWithRoute(FString Subroute);
};
