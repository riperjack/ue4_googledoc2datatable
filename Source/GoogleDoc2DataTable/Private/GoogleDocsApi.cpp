#include "GoogleDocsApi.h"

const FString UGoogleDocsApi::ApiBaseUrl = "https://docs.google.com/spreadsheets/d/";

TSharedRef<IHttpRequest,ESPMode::ThreadSafe> UGoogleDocsApi::RequestWithRoute(FString Subroute) 
{
	Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest,ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetURL(UGoogleDocsApi::ApiBaseUrl + Subroute);
	SetRequestHeaders(Request);
	return Request;
}

void UGoogleDocsApi::SetRequestHeaders(TSharedRef<IHttpRequest,ESPMode::ThreadSafe>& Request) 
{
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("text/csv"));
	Request->SetHeader(TEXT("Accepts"), TEXT("text/csv"));
}

TSharedRef<IHttpRequest,ESPMode::ThreadSafe> UGoogleDocsApi::GetRequest(FString Subroute) 
{
	TSharedRef<IHttpRequest,ESPMode::ThreadSafe> Request = RequestWithRoute(Subroute);
	Request->SetVerb("GET");
	return Request;
}

TSharedRef<IHttpRequest,ESPMode::ThreadSafe> UGoogleDocsApi::PostRequest(FString Subroute, FString ContentJsonString) 
{
	TSharedRef<IHttpRequest,ESPMode::ThreadSafe> Request = RequestWithRoute(Subroute);
	Request->SetVerb("POST");
	Request->SetContentAsString(ContentJsonString);
	return Request;
}

bool UGoogleDocsApi::ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful) 
{
	if (!bWasSuccessful || !Response.IsValid()) return false;
	if (EHttpResponseCodes::IsOk(Response->GetResponseCode())) return true;
	else {
		UE_LOG(LogTemp, Error, TEXT("Http Response returned error code: %d"), Response->GetResponseCode());
		return false;
	}
}

void UGoogleDocsApi::ProcessResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) 
{
	if (!ResponseIsValid(Response, bWasSuccessful)) return;

	OnResponseDelegate.ExecuteIfBound(Response->GetContentAsString());
}

void UGoogleDocsApi::SendRequest(FString docId) 
{
	TSharedRef<IHttpRequest,ESPMode::ThreadSafe> Request = GetRequest(FString::Printf(TEXT("%s/export?format=csv"), *docId));
	Request->OnProcessRequestComplete().BindUObject(this, &UGoogleDocsApi::ProcessResponse);
	Request->ProcessRequest();
}