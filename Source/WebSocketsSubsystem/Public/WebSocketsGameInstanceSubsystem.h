// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WebSocketsGameInstanceSubsystem.generated.h"

class UWebSocket;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWebSocketCreated, FName, WebSocketName, UWebSocket*, WebSocket);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketDestroyed, FName, WebSocketName);

UCLASS(meta = (DisplayName = "WebSockets Subsystem", ToolTip = "Manages named WebSocket connections for the current game instance."))
class WEBSOCKETSSUBSYSTEM_API UWebSocketsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

#pragma region Events

    // Broadcast when a WebSocket wrapper has been created and registered.
    UPROPERTY(BlueprintAssignable, Category = "WebSockets|Events")
    FOnWebSocketCreated OnWebSocketCreated;

    // Broadcast when a WebSocket has completed closure and has been unregistered.
    UPROPERTY(BlueprintAssignable, Category = "WebSockets|Events")
    FOnWebSocketDestroyed OnWebSocketDestroyed;

#pragma endregion

#pragma region Lifecycle

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

#pragma endregion

private:

#pragma region Internal State

    // Maps WebSocket names to their corresponding wrapper objects.
    UPROPERTY()
    TMap<FName, TObjectPtr<UWebSocket>> WebSockets;

    // WebSockets whose close operation has been requested but not completed.
    TSet<FName> PendingDestruction;

    UFUNCTION()
    void HandleWebSocketClosed(UWebSocket* InWebSocket);

#pragma endregion

public:

#pragma region WebSocket Management

    // Creates and registers a WebSocket wrapper.
    UFUNCTION(BlueprintCallable, Category = "WebSockets", meta = (DisplayName = "Create WebSocket", ToolTip = "Creates and registers a WebSocket wrapper. This does not connect to the server."))
    bool CreateWebSocket(FName InWebSocketName, const FString& InServerURL, UWebSocket*& OutWebSocket, FString& OutMessage);

    // Creates and registers a WebSocket wrapper with custom HTTP upgrade headers.
    UFUNCTION(BlueprintCallable, Category = "WebSockets", meta = (DisplayName = "Create WebSocket with Headers", ToolTip = "Creates and registers a WebSocket wrapper with custom HTTP upgrade headers. This does not connect to the server."))
    bool CreateWebSocketWithHeaders(FName InWebSocketName, const FString& InServerURL, const TMap<FString, FString>& InUpgradeHeaders, UWebSocket*& OutWebSocket, FString& OutMessage);

    // Returns the WebSocket wrapper registered under the specified name.
    UFUNCTION(BlueprintCallable, Category = "WebSockets", meta = (DisplayName = "Get WebSocket", ToolTip = "Returns the WebSocket wrapper registered under the specified name."))
    UWebSocket* GetWebSocket(FName InWebSocketName) const;

    // Returns a copy of all currently registered WebSocket wrappers.
    UFUNCTION(BlueprintPure, Category = "WebSockets", meta = (DisplayName = "Get All WebSockets", ToolTip = "Returns a copy of all currently registered WebSocket wrappers."))
    TMap<FName, UWebSocket*> GetAllWebSockets() const;

    // Returns all registered WebSockets without creating a copy. C++ only.
    const TMap<FName, TObjectPtr<UWebSocket>>& GetAllWebSockets_Const() const;

    // Requests closure and unregisters the WebSocket after its close event is received.
    UFUNCTION(BlueprintCallable, Category = "WebSockets", meta = (DisplayName = "Destroy WebSocket", ToolTip = "Requests closure and unregisters the WebSocket after its close event is received."))
    bool DestroyWebSocket(FName InWebSocketName, FString& OutMessage);

    // Requests closure of all registered WebSockets.
    UFUNCTION(BlueprintCallable, Category = "WebSockets", meta = (DisplayName = "Destroy All WebSockets", ToolTip = "Requests closure of all registered WebSockets. Removal completes asynchronously as each close event is received."))
    bool DestroyAllWebSockets(FString& OutMessage);

#pragma endregion
};
