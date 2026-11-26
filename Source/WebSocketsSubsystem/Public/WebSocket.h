// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WebSocket.generated.h"

class IWebSocket;
class UWebSocket;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketConnectionError, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWebSocketClosed, int32, StatusCode, const FString&, Reason, bool, bWasClean);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketMessageReceived, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketMessageSent, const FString&, Message);

// Internal notification used by the subsystem to complete deferred destruction.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketClosedInternal, UWebSocket*, WebSocket);

UCLASS(BlueprintType, meta = (DisplayName = "WebSocket", ToolTip = "A wrapper around a single WebSocket connection."))
class WEBSOCKETSSUBSYSTEM_API UWebSocket final : public UObject
{
    GENERATED_BODY()

    friend class UWebSocketsSubsystem;

public:

#pragma region Events

    // Broadcast when the WebSocket connection is established.
    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    FOnWebSocketConnected OnWebSocketConnected;

    // Broadcast when the WebSocket connection encounters an error.
    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    FOnWebSocketConnectionError OnWebSocketConnectionError;

    // Broadcast when the WebSocket connection is closed.
    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    FOnWebSocketClosed OnWebSocketClosed;

    // Broadcast when a message is received.
    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    FOnWebSocketMessageReceived OnWebSocketMessageReceived;

    // Broadcast when a message has been sent.
    UPROPERTY(BlueprintAssignable, Category = "WebSocket|Events")
    FOnWebSocketMessageSent OnWebSocketMessageSent;

#pragma endregion

#pragma region Initialization

    // Initializes this wrapper with an existing WebSocket implementation.
    bool InitWebSocket(TSharedPtr<IWebSocket> InWebSocket);

    virtual void BeginDestroy() override;

#pragma endregion

#pragma region Connection

    // Starts connecting to the WebSocket server.
    UFUNCTION(BlueprintCallable, Category = "WebSocket", meta = (ToolTip = "Starts connecting to the WebSocket server."))
    void Connect();

    // Requests closure of the WebSocket connection.
    UFUNCTION(BlueprintCallable, Category = "WebSocket", meta = (ToolTip = "Requests closure of the WebSocket connection. The close operation may complete asynchronously."))
    void Close(int32 InStatusCode = 1000, const FString& InReason = TEXT(""));

    // Returns whether the wrapper has an initialized WebSocket implementation.
    UFUNCTION(BlueprintPure, Category = "WebSocket", meta = (ToolTip = "Returns whether the wrapper has an initialized WebSocket implementation."))
    bool IsInitialized() const;

    // Returns whether the WebSocket connection is currently connected.
    UFUNCTION(BlueprintPure, Category = "WebSocket", meta = (ToolTip = "Returns whether the WebSocket connection is currently connected."))
    bool IsConnected() const;

    // Sends a text message when the WebSocket is connected.
    UFUNCTION(BlueprintCallable, Category = "WebSocket", meta = (ToolTip = "Sends a text message when the WebSocket is connected."))
    void SendMessage(const FString& InMessage);

#pragma endregion

private:

#pragma region Internal State

    // The underlying Unreal Engine WebSocket implementation.
    TSharedPtr<IWebSocket> InternalWebSocket;

    // Tracks whether the underlying WebSocket has emitted its close event.
    bool bHasClosed = false;

    // Internal close notification consumed by the owning subsystem.
    UPROPERTY()
    FOnWebSocketClosedInternal OnWebSocketClosedInternal;

#pragma endregion

#pragma region Internal Handlers

    UFUNCTION()
    void OnWebSocketConnected_Internal();

    UFUNCTION()
    void OnWebSocketConnectionError_Internal(const FString& InError);

    UFUNCTION()
    void OnWebSocketClosed_Internal(int32 InStatusCode, const FString& InReason, bool bInWasClean);

    UFUNCTION()
    void OnWebSocketMessageReceived_Internal(const FString& InMessage);

    UFUNCTION()
    void OnWebSocketMessageSent_Internal(const FString& InMessage);

#pragma endregion
};