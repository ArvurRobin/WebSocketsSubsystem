// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#include "WebSocketsGameInstanceSubsystem.h"
#include "WebSocket.h"

#include "WebSocketsModule.h"
#include "IWebSocket.h"

void UWebSocketsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UWebSocketsSubsystem::Deinitialize()
{
    // Clean up WebSocket connections.

    Super::Deinitialize();
}

bool UWebSocketsSubsystem::CreateWebSocket(FName WebSocketName, const FString& ServerURL, UWebSocket*& OutWebSocket, FString& OutMessage)
{
    return CreateWebSocketWithHeaders(WebSocketName, ServerURL, TMap<FString, FString>() ,OutWebSocket, OutMessage);
}

bool UWebSocketsSubsystem::CreateWebSocketWithHeaders(FName WebSocketName, const FString& ServerURL, const TMap<FString, FString>& UpgradeHeaders, UWebSocket*& OutWebSocket, FString& OutMessage)
{
    // Reset output parameters.
    OutWebSocket = nullptr;
    OutMessage.Reset();

    // Validate WebSocket name.
    if (WebSocketName.IsNone())
    {
        OutMessage = TEXT("WebSocket name cannot be None.");
        return false;
    }

    // Validate server URL.
    if (ServerURL.IsEmpty())
    {
        OutMessage = TEXT("Server URL cannot be empty.");
        return false;
    }

    if (!ServerURL.StartsWith(TEXT("ws://"), ESearchCase::IgnoreCase) &&
        !ServerURL.StartsWith(TEXT("wss://"), ESearchCase::IgnoreCase))
    {
        OutMessage = TEXT("Invalid WebSocket URL. URL must start with 'ws://' or 'wss://'.");
        return false;
    }

    // Check for duplicate WebSocket names.
    if (WebSockets.Contains(WebSocketName))
    {
        OutMessage = FString::Printf(
            TEXT("A WebSocket with the name '%s' already exists."),
            *WebSocketName.ToString()
        );
        return false;
    }

    // Create the actual WebSocket connection.
    TSharedRef<IWebSocket> ActualSocket =
        FWebSocketsModule::Get().CreateWebSocket(
            ServerURL,
            TArray<FString>(),
            UpgradeHeaders
        );

    // Create the UObject wrapper.
    UWebSocket* WrapperSocket = NewObject<UWebSocket>(this);

    // Initialize the wrapper.
    WrapperSocket->InitWebSocket(ActualSocket.ToSharedPtr());

    // Register the WebSocket.
    WebSockets.Add(WebSocketName, WrapperSocket);

    // Set output parameters.
    OutWebSocket = WrapperSocket;
    OutMessage = TEXT("WebSocket created successfully.");

    OnWebSocketCreated.Broadcast(WebSocketName, WrapperSocket);

    return true;
}

UWebSocket* UWebSocketsSubsystem::GetWebSocket(FName WebSocketName) const
{
    if (const TObjectPtr<UWebSocket>* FoundWebSocket = WebSockets.Find(WebSocketName))
    {
        return FoundWebSocket->Get();
    }

    return nullptr;
}

TMap<FName, UWebSocket*> UWebSocketsSubsystem::GetAllWebSockets() const
{
    TMap<FName, UWebSocket*> Result;

    for (const TPair<FName, TObjectPtr<UWebSocket>>& Pair : WebSockets)
    {
        Result.Add(Pair.Key, Pair.Value.Get());
    }

    return Result;
}

const TMap<FName, TObjectPtr<UWebSocket>>& UWebSocketsSubsystem::GetAllWebSockets_Const() const
{
    return WebSockets;
}

bool UWebSocketsSubsystem::DestroyWebSocket(FName WebSocketName, FString& OutMessage)
{
    OutMessage.Reset();

    // Check whether the WebSocket exists.
    TObjectPtr<UWebSocket>* FoundWebSocket = WebSockets.Find(WebSocketName);

    if (!FoundWebSocket)
    {
        OutMessage = FString::Printf(
            TEXT("Failed to destroy WebSocket: No WebSocket with the name '%s' exists."),
            *WebSocketName.ToString()
        );

        return false;
    }

    // Close the connection before removing it.
    UWebSocket* WebSocket = FoundWebSocket->Get();

    if (IsValid(WebSocket))
    {
        WebSocket->Close(1000, TEXT("WebSocket destroyed"));
    }

    // Remove the WebSocket from the map.
    WebSockets.Remove(WebSocketName);

    OutMessage = FString::Printf(
        TEXT("WebSocket '%s' was successfully removed and its connection was closed."),
        *WebSocketName.ToString()
    );

    OnWebSocketDestroyed.Broadcast(WebSocketName);

    return true;
}

bool UWebSocketsSubsystem::DestroyAllWebSockets(FString& OutMessage)
{
    OutMessage.Reset();

    // Check whether there are any WebSockets to destroy.
    if (WebSockets.IsEmpty())
    {
        OutMessage = TEXT("There are no WebSockets to destroy.");
        return false;
    }

    const int32 WebSocketCount = WebSockets.Num();

    // Close all existing WebSocket connections.
    for (const TPair<FName, TObjectPtr<UWebSocket>>& Pair : WebSockets)
    {
        UWebSocket* WebSocket = Pair.Value.Get();

        if (IsValid(WebSocket))
        {
            WebSocket->Close(1000, TEXT("All WebSockets destroyed"));
            WebSockets.Remove(Pair.Key);
            OnWebSocketDestroyed.Broadcast(Pair.Key);
        }
    }

    // Clear the map.
    WebSockets.Empty();

    OutMessage = FString::Printf(
        TEXT("Successfully removed %d WebSocket(s) and initiated connection closure."),
        WebSocketCount
    );

    return true;
}