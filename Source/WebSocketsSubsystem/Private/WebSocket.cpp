// Copyright © 2026 Robin Hasenbach. All Rights Reserved.


// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#include "WebSocket.h"
#include "IWebSocket.h"

bool UWebSocket::InitWebSocket(TSharedPtr<IWebSocket> InWebSocket)
{
    if (!InWebSocket.IsValid())
    {
        return false;
    }

    if (InternalWebSocket.IsValid())
    {
        InternalWebSocket->OnConnected().RemoveAll(this);
        InternalWebSocket->OnConnectionError().RemoveAll(this);
        InternalWebSocket->OnClosed().RemoveAll(this);
        InternalWebSocket->OnMessage().RemoveAll(this);
        InternalWebSocket->OnMessageSent().RemoveAll(this);
    }

    InternalWebSocket = MoveTemp(InWebSocket);

    InternalWebSocket->OnConnected().AddUObject(this, &ThisClass::OnWebSocketConnected_Internal);
    InternalWebSocket->OnConnectionError().AddUObject(this, &ThisClass::OnWebSocketConnectionError_Internal);
    InternalWebSocket->OnClosed().AddUObject(this, &ThisClass::OnWebSocketClosed_Internal);
    InternalWebSocket->OnMessage().AddUObject(this, &ThisClass::OnWebSocketMessageReceived_Internal);
    InternalWebSocket->OnMessageSent().AddUObject(this, &ThisClass::OnWebSocketMessageSent_Internal);

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
    if (InternalWebSocket.IsValid())
    {
        InternalWebSocket->Connect();
    }
}

void UWebSocket::Close(int32 StatusCode, const FString& Reason)
{
    if (InternalWebSocket.IsValid())
    {
        InternalWebSocket->Close(StatusCode, Reason);
    }
}

bool UWebSocket::IsConnected() const
{
    return InternalWebSocket.IsValid() && InternalWebSocket->IsConnected();
}

void UWebSocket::SendMessage(const FString& Message)
{
    if (InternalWebSocket.IsValid() && InternalWebSocket->IsConnected())
    {
        InternalWebSocket->Send(Message);
    }
}

void UWebSocket::OnWebSocketConnected_Internal()
{
    OnWebSocketConnected.Broadcast();
}

void UWebSocket::OnWebSocketConnectionError_Internal(const FString& Error)
{
    OnWebSocketConnectionError.Broadcast(Error);
}

void UWebSocket::OnWebSocketClosed_Internal(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    OnWebSocketClosed.Broadcast(StatusCode, Reason, bWasClean);
}

void UWebSocket::OnWebSocketMessageReceived_Internal(const FString& Message)
{
    OnWebSocketMessageReceived.Broadcast(Message);
}

void UWebSocket::OnWebSocketMessageSent_Internal(const FString& Message)
{
    OnWebSocketMessageSent.Broadcast(Message);
}
