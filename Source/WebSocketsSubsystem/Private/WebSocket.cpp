// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#include "WebSocket.h"
#include "IWebSocket.h"

bool UWebSocket::InitWebSocket(TSharedPtr<IWebSocket> InWebSocket)
{
    if (!InWebSocket.IsValid())
    {
        return false;
    }

    // Prevent replacing an initialized connection without closing it first.
    if (InternalWebSocket.IsValid())
    {
        return false;
    }

    InternalWebSocket = MoveTemp(InWebSocket);
    bHasClosed = false;

    InternalWebSocket->OnConnected().AddUObject(this, &UWebSocket::OnWebSocketConnected_Internal);
    InternalWebSocket->OnConnectionError().AddUObject(this, &UWebSocket::OnWebSocketConnectionError_Internal);
    InternalWebSocket->OnClosed().AddUObject(this, &UWebSocket::OnWebSocketClosed_Internal);
    InternalWebSocket->OnMessage().AddUObject(this, &UWebSocket::OnWebSocketMessageReceived_Internal);
    InternalWebSocket->OnMessageSent().AddUObject(this, &UWebSocket::OnWebSocketMessageSent_Internal);

    return true;
}

void UWebSocket::BeginDestroy()
{
    if (InternalWebSocket.IsValid())
    {
        InternalWebSocket->OnConnected().RemoveAll(this);
        InternalWebSocket->OnConnectionError().RemoveAll(this);
        InternalWebSocket->OnClosed().RemoveAll(this);
        InternalWebSocket->OnMessage().RemoveAll(this);
        InternalWebSocket->OnMessageSent().RemoveAll(this);

        InternalWebSocket.Reset();
    }

    Super::BeginDestroy();
}

void UWebSocket::Connect()
{
    if (IsInitialized() && !bHasClosed)
    {
        InternalWebSocket->Connect();
    }
}

void UWebSocket::Close(int32 InStatusCode, const FString& InReason)
{
    if (IsInitialized() && !bHasClosed)
    {
        InternalWebSocket->Close(InStatusCode, InReason);
    }
}

bool UWebSocket::IsInitialized() const
{
    return InternalWebSocket.IsValid();
}

bool UWebSocket::IsConnected() const
{
    return IsInitialized() && !bHasClosed && InternalWebSocket->IsConnected();
}

void UWebSocket::SendMessage(const FString& InMessage)
{
    if (IsConnected())
    {
        InternalWebSocket->Send(InMessage);
    }
}

void UWebSocket::OnWebSocketConnected_Internal()
{
    if (!bHasClosed)
    {
        OnWebSocketConnected.Broadcast();
    }
}

void UWebSocket::OnWebSocketConnectionError_Internal(const FString& InError)
{
    OnWebSocketConnectionError.Broadcast(InError);
}

void UWebSocket::OnWebSocketClosed_Internal(int32 InStatusCode, const FString& InReason, bool bInWasClean)
{
    if (bHasClosed)
    {
        return;
    }

    bHasClosed = true;

    OnWebSocketClosed.Broadcast(InStatusCode, InReason, bInWasClean);
    OnWebSocketClosedInternal.Broadcast(this);
}

void UWebSocket::OnWebSocketMessageReceived_Internal(const FString& InMessage)
{
    if (!bHasClosed)
    {
        OnWebSocketMessageReceived.Broadcast(InMessage);
    }
}

void UWebSocket::OnWebSocketMessageSent_Internal(const FString& InMessage)
{
    if (!bHasClosed)
    {
        OnWebSocketMessageSent.Broadcast(InMessage);
    }
}