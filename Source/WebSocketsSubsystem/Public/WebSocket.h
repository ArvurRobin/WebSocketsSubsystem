// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WebSocket.generated.h"

class IWebSocket;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketConnectionError, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWebSocketClosed, int32, StatusCode, const FString&, Reason, bool, bWasClean);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketMessageReceived, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketMessageSent, const FString&, Message);

UCLASS(
	BlueprintType,
	DisplayName = "WebSocket",
	Category = "WebSocket",
	meta = (ToolTip = "A single WebSocket Object.")
)
class WEBSOCKETSSUBSYSTEM_API UWebSocket final : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnWebSocketConnected OnWebSocketConnected;

	UPROPERTY(BlueprintAssignable)
	FOnWebSocketConnectionError OnWebSocketConnectionError;

	UPROPERTY(BlueprintAssignable)
	FOnWebSocketClosed OnWebSocketClosed;

	UPROPERTY(BlueprintAssignable)
	FOnWebSocketMessageReceived OnWebSocketMessageReceived;

	UPROPERTY(BlueprintAssignable)
	FOnWebSocketMessageSent OnWebSocketMessageSent;

	bool InitWebSocket(TSharedPtr<IWebSocket> InWebSocket);

	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void Connect();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void Close(int32 StatusCode = 1000, const FString& Reason = TEXT(""));

	UFUNCTION(BlueprintPure, Category = "WebSocket")
	bool IsInitialized() const;

	UFUNCTION(BlueprintPure, Category = "WebSocket")
	bool IsConnected() const;

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendMessage(const FString& Message);

private:

	UFUNCTION()
	void OnWebSocketConnected_Internal();

	UFUNCTION()
	void OnWebSocketConnectionError_Internal(const FString& Error);

	UFUNCTION()
	void OnWebSocketClosed_Internal(int32 StatusCode, const FString& Reason, bool bWasClean);

	UFUNCTION()
	void OnWebSocketMessageReceived_Internal(const FString& Message);

	UFUNCTION()
	void OnWebSocketMessageSent_Internal(const FString& Message);

	TSharedPtr<IWebSocket> InternalWebSocket;
};
