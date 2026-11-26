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
    // Request closure of all remaining WebSocket connections.
    FString OutMessage;
    DestroyAllWebSockets(OutMessage);

    Super::Deinitialize();
}

bool UWebSocketsSubsystem::CreateWebSocket(FName InWebSocketName, const FString& InServerURL, UWebSocket*& OutWebSocket, FString& OutMessage)
{
    return CreateWebSocketWithHeaders(InWebSocketName, InServerURL, TMap<FString, FString>(), OutWebSocket, OutMessage);
}

bool UWebSocketsSubsystem::CreateWebSocketWithHeaders(FName InWebSocketName, const FString& InServerURL, const TMap<FString, FString>& InUpgradeHeaders, UWebSocket*& OutWebSocket, FString& OutMessage)
{
    // Reset output parameters.
    OutWebSocket = nullptr;
    OutMessage.Reset();

    // Validate WebSocket name.
    if (InWebSocketName.IsNone())
    {
        OutMessage = TEXT("WebSocket name cannot be None.");
        return false;
    }

    // Validate server URL.
    if (InServerURL.IsEmpty())
    {
        OutMessage = TEXT("Server URL cannot be empty.");
        return false;
    }

    if (!InServerURL.StartsWith(TEXT("ws://"), ESearchCase::IgnoreCase) &&
        !InServerURL.StartsWith(TEXT("wss://"), ESearchCase::IgnoreCase))
    {
        OutMessage = TEXT("Invalid WebSocket URL. URL must start with 'ws://' or 'wss://'.");
        return false;
    }

    // Check for duplicate WebSocket names.
    if (WebSockets.Contains(InWebSocketName))
    {
        OutMessage = FString::Printf(
            TEXT("A WebSocket with the name '%s' already exists."),
            *InWebSocketName.ToString()
        );

        return false;
    }

    // Create the actual WebSocket connection.
    TSharedRef<IWebSocket> ActualSocket =
        FWebSocketsModule::Get().CreateWebSocket(
            InServerURL,
            TArray<FString>(),
            InUpgradeHeaders
        );

    // Create and initialize the UObject wrapper.
    UWebSocket* WrapperSocket = NewObject<UWebSocket>(this);

    if (!IsValid(WrapperSocket) || !WrapperSocket->InitWebSocket(ActualSocket))
    {
        OutMessage = TEXT("Failed to initialize the WebSocket wrapper.");
        return false;
    }

    // Listen for the wrapper's close event so it remains registered until closure completes.
    WrapperSocket->OnWebSocketClosedInternal.AddUniqueDynamic(
        this, &ThisClass::HandleWebSocketClosed
    );

    // Register the WebSocket.
    WebSockets.Add(InWebSocketName, WrapperSocket);

    // Set output parameters.
    OutWebSocket = WrapperSocket;
    OutMessage = TEXT("WebSocket created successfully.");

    OnWebSocketCreated.Broadcast(InWebSocketName, WrapperSocket);

    return true;
}

UWebSocket* UWebSocketsSubsystem::GetWebSocket(FName InWebSocketName) const
{
    if (const TObjectPtr<UWebSocket>* FoundWebSocket = WebSockets.Find(InWebSocketName))
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

bool UWebSocketsSubsystem::DestroyWebSocket(FName InWebSocketName, FString& OutMessage)
{
    OutMessage.Reset();

    TObjectPtr<UWebSocket>* FoundWebSocket = WebSockets.Find(InWebSocketName);

    if (!FoundWebSocket)
    {
        OutMessage = FString::Printf(
            TEXT("Failed to destroy WebSocket: No WebSocket with the name '%s' exists."),
            *InWebSocketName.ToString()
        );

        return false;
    }

    // Avoid issuing multiple close requests for the same WebSocket.
    if (PendingDestruction.Contains(InWebSocketName))
    {
        OutMessage = FString::Printf(
            TEXT("WebSocket '%s' is already pending destruction."),
            *InWebSocketName.ToString()
        );

        return true;
    }

    UWebSocket* WebSocket = FoundWebSocket->Get();

    // Remove invalid wrappers immediately.
    if (!IsValid(WebSocket))
    {
        WebSockets.Remove(InWebSocketName);
        PendingDestruction.Remove(InWebSocketName);

        OutMessage = FString::Printf(
            TEXT("Invalid WebSocket '%s' was removed."),
            *InWebSocketName.ToString()
        );

        OnWebSocketDestroyed.Broadcast(InWebSocketName);
        return true;
    }

    // If the close event has already occurred, removal can happen immediately.
    if (WebSocket->bHasClosed)
    {
        WebSockets.Remove(InWebSocketName);
        PendingDestruction.Remove(InWebSocketName);

        OutMessage = FString::Printf(
            TEXT("WebSocket '%s' was already closed and has been removed."),
            *InWebSocketName.ToString()
        );

        OnWebSocketDestroyed.Broadcast(InWebSocketName);
        return true;
    }

    // Keep the wrapper registered until its asynchronous close event arrives.
    PendingDestruction.Add(InWebSocketName);
    WebSocket->Close(1000, TEXT("WebSocket destroyed"));

    OutMessage = FString::Printf(
        TEXT("Closure requested for WebSocket '%s'. It will be removed after the close event."),
        *InWebSocketName.ToString()
    );

    return true;
}

bool UWebSocketsSubsystem::DestroyAllWebSockets(FString& OutMessage)
{
    OutMessage.Reset();

    if (WebSockets.IsEmpty())
    {
        OutMessage = TEXT("There are no WebSockets to destroy.");
        return false;
    }

    // Copy the names because DestroyWebSocket may remove entries synchronously.
    TArray<FName> WebSocketNames;
    WebSockets.GetKeys(WebSocketNames);

    int32 RequestedCount = 0;

    for (const FName WebSocketName : WebSocketNames)
    {
        FString DestroyMessage;

        if (DestroyWebSocket(WebSocketName, DestroyMessage))
        {
            ++RequestedCount;
        }
    }

    OutMessage = FString::Printf(
        TEXT("Closure requested for %d WebSocket(s). Removal will complete as close events arrive."),
        RequestedCount
    );

    return RequestedCount > 0;
}

void UWebSocketsSubsystem::HandleWebSocketClosed(UWebSocket* InWebSocket)
{
    if (!IsValid(InWebSocket))
    {
        return;
    }

    // Find the registered name for the wrapper that has just closed.
    FName ClosedWebSocketName = NAME_None;

    for (const TPair<FName, TObjectPtr<UWebSocket>>& Pair : WebSockets)
    {
        if (Pair.Value.Get() == InWebSocket)
        {
            ClosedWebSocketName = Pair.Key;
            break;
        }
    }

    if (ClosedWebSocketName.IsNone() || !PendingDestruction.Contains(ClosedWebSocketName))
    {
        return;
    }

    WebSockets.Remove(ClosedWebSocketName);
    PendingDestruction.Remove(ClosedWebSocketName);

    OnWebSocketDestroyed.Broadcast(ClosedWebSocketName);
}
