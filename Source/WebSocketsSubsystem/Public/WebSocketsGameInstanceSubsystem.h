// Copyright © 2026 Robin Hasenbach. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WebSocketsGameInstanceSubsystem.generated.h"

class UWebSocket;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWebSocketCreated, FName, WebSocketName, UWebSocket*, WebSocket);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketDestroyed, FName, WebSocketName);

UCLASS(
    DisplayName = "WebSockets Subsystem",
    Category = "WebSockets",
    meta = (ToolTip = "Manages WebSocket connections.")
)
class WEBSOCKETSSUBSYSTEM_API UWebSocketsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "WebSockets")
    FOnWebSocketCreated OnWebSocketCreated;

    UPROPERTY(BlueprintAssignable, Category = "WebSockets")
    FOnWebSocketDestroyed OnWebSocketDestroyed;

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    UPROPERTY()
    TMap<FName, TObjectPtr<UWebSocket>> WebSockets;

public:
    UFUNCTION(BlueprintCallable, Category = "WebSockets", meta = (DisplayName = "Create WebSocket"))
    bool CreateWebSocket(FName WebSocketName, const FString& ServerURL, UWebSocket*& OutWebSocket, FString& OutMessage);

    UFUNCTION(BlueprintCallable, Category = "WebSockets", meta = (DisplayName = "Create WebSocket with Headers"))
    bool CreateWebSocketWithHeaders(FName WebSocketName, const FString& ServerURL, const TMap<FString, FString>& UpgradeHeaders, UWebSocket*& OutWebSocket, FString& OutMessage);

    UFUNCTION(BlueprintCallable, Category = "WebSockets", meta = (DisplayName = "Get WebSocket"))
    UWebSocket* GetWebSocket(FName WebSocketName) const;

    UFUNCTION(BlueprintPure, Category = "WebSockets", meta = (DisplayName = "Get All WebSockets"))
    TMap<FName, UWebSocket*> GetAllWebSockets() const;

    // More efficient but C++ only version of GetAllWebSockets
    const TMap<FName, TObjectPtr<UWebSocket>>& GetAllWebSockets_Const() const;

    UFUNCTION(BlueprintCallable, Category = "WebSockets", meta = (DisplayName = "Destroy WebSocket"))
    bool DestroyWebSocket(FName WebSocketName, FString& OutMessage);

    UFUNCTION(BlueprintCallable, Category = "WebSockets", meta = (DisplayName = "Destroy All WebSockets"))
    bool DestroyAllWebSockets(FString& OutMessage);
};