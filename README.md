# WebSocketsSubsystem

**WebSocketsSubsystem** is an Unreal Engine C++ plugin that provides a centralized way to create, manage, and communicate through multiple WebSocket connections.

Built around an Unreal Engine `UGameInstanceSubsystem` and a Blueprint-accessible `UWebSocket` wrapper, it makes WebSocket communication available to both C++ and Blueprint without requiring each system to manage its own low-level WebSocket implementation.

## Features

- **Multiple connections:** Create and manage multiple WebSocket connections, each identified by a unique name.
- **Blueprint support:** Create, access, connect, communicate through, and destroy WebSockets using Blueprint nodes.
- **C++ API:** Access the subsystem and WebSocket objects directly from C++.
- **Custom headers:** Create connections with custom HTTP upgrade headers.
- **Event-driven communication:** Receive events when a connection is established, closed, or encounters an error, and when messages are sent or received.
- **Connection management:** Retrieve individual WebSockets, get all registered connections, or destroy connections individually or all at once.
- **Automatic cleanup:** The subsystem manages the lifetime of its WebSocket objects and requests connection closure during shutdown.

## Requirements

- Unreal Engine with the built-in `WebSockets` module.
- A WebSocket-compatible server or service.

No additional third-party libraries are required.

## Installation

1. Download or clone this repository.
2. Copy the `WebSocketsSubsystem` plugin folder into your project's `Plugins` directory.

   Your directory structure should look similar to this:

   ```text
   YourProject/
   └── Plugins/
       └── WebSocketsSubsystem/
           ├── WebSocketsSubsystem.uplugin
           └── Source/
               └── WebSocketsSubsystem/
   ```

3. Open your Unreal Engine project.
4. Enable **WebSockets Subsystem** in the Plugins window if it is not already enabled.
5. Restart the editor if necessary.

If you are using C++ and the plugin was added after your project was generated, regenerate your project files and rebuild the project.

## Getting Started

The plugin provides two main classes:

| Class | Description |
|---|---|
| `UWebSocketsSubsystem` | Manages and stores WebSocket connections for the current GameInstance. |
| `UWebSocket` | Wraps an individual WebSocket connection and exposes its functionality and events. |

A connection is created through the subsystem and returned as a `UWebSocket` object. You can then use that object to connect to a server, send messages, and listen for events.

### Blueprint

The subsystem is accessible through the Game Instance Subsystem functionality in Blueprint.

A typical connection workflow looks like this:

1. Get the **WebSockets Subsystem**.
2. Call **Create WebSocket** and provide a unique name and server URL.
3. Check the returned success value and output message.
4. Use the returned WebSocket object to bind to its events.
5. Call **Connect** to establish the connection.
6. Use **Send Message** to send data once connected.
7. Call **Destroy WebSocket** when the connection is no longer needed.

For servers that require additional HTTP upgrade headers, use **Create WebSocket with Headers** instead.

### C++

Include the subsystem and WebSocket headers:

```cpp
#include "WebSocketsGameInstanceSubsystem.h"
#include "WebSocket.h"
```

You can access the subsystem through your GameInstance:

```cpp
UWebSocketsSubsystem* WebSocketsSubsystem = GetGameInstance()->GetSubsystem<UWebSocketsSubsystem>();

if (!IsValid(WebSocketsSubsystem))
{
    return;
}

UWebSocket* WebSocket = nullptr;
FString OutMessage;

if (WebSocketsSubsystem->CreateWebSocket(
    TEXT("MyConnection"),
    TEXT("wss://example.com"),
    WebSocket,
    OutMessage))
{
    WebSocket->Connect();
}
else
{
    UE_LOG(LogTemp, Error, TEXT("Failed to create WebSocket: %s"), *OutMessage);
}
```

The example creates a connection named `MyConnection`. The name must be unique among the connections managed by the subsystem.

For a server requiring custom HTTP upgrade headers, use `CreateWebSocketWithHeaders()`:

```cpp
TMap<FString, FString> Headers;
Headers.Add(TEXT("Authorization"), TEXT("Bearer YOUR_TOKEN"));

UWebSocket* WebSocket = nullptr;
FString OutMessage;

if (WebSocketsSubsystem->CreateWebSocketWithHeaders(
    TEXT("AuthenticatedConnection"),
    TEXT("wss://example.com"),
    Headers,
    WebSocket,
    OutMessage))
{
    WebSocket->Connect();
}
```

Replace the example URL and header values with those required by your server.

## WebSocket Events

Each `UWebSocket` object exposes the following Blueprint-assignable events:

| Event | Description |
|---|---|
| `OnWebSocketConnected` | Fired when the connection is successfully established. |
| `OnWebSocketConnectionError` | Fired when a connection error occurs. Provides the error message. |
| `OnWebSocketClosed` | Fired when the connection closes. Provides the status code, reason, and whether the closure was clean. |
| `OnWebSocketMessageReceived` | Fired when a message is received from the server. |
| `OnWebSocketMessageSent` | Fired when a message is sent. |

You should use these events to react to connection changes and incoming or outgoing messages.

**Important:** Calling `Connect()` does not mean the connection is already established. Wait for `OnWebSocketConnected` before sending messages.

## Sending and Receiving Messages

Once connected, you can send a message using:

```cpp
WebSocket->SendMessage(TEXT("Hello Server!"));
```

Incoming messages are delivered through `OnWebSocketMessageReceived`.

The plugin exposes messages as `FString` values. If your server uses JSON, you can use Unreal Engine's built-in `Json` and `JsonUtilities` modules to serialize and deserialize the message contents.

The plugin does not impose a specific message format or communication protocol. You are free to use whatever format your server expects.

## Managing Connections

The subsystem provides several functions for managing registered WebSockets.

| Function | Description |
|---|---|
| `CreateWebSocket()` | Creates a WebSocket connection using a name and URL. |
| `CreateWebSocketWithHeaders()` | Creates a WebSocket connection with custom upgrade headers. |
| `GetWebSocket()` | Retrieves a WebSocket by its registered name. |
| `GetAllWebSockets()` | Returns all registered WebSockets. |
| `DestroyWebSocket()` | Requests closure and removal of a specific WebSocket. |
| `DestroyAllWebSockets()` | Requests closure and removal of all registered WebSockets. |

### Destroying a WebSocket

When you call `DestroyWebSocket()`, the subsystem requests that the underlying connection closes.

Removal is asynchronous: the WebSocket remains registered until its close event is received. Once closed and removed, `OnWebSocketDestroyed` is broadcast by the subsystem.

This means that a successful return value indicates that the destruction request was accepted, not necessarily that the connection has already been fully closed.

The same behavior applies when using `DestroyAllWebSockets()`.

## Notes

- WebSocket connections are managed by the GameInstance subsystem. They are therefore scoped to the lifetime of the GameInstance.
- Each connection must have a unique `FName` within the subsystem.
- Both `ws://` and `wss://` URLs are supported.
- Sending messages is only possible while the WebSocket is connected.
- The plugin provides the connection-management layer; it does not implement application-specific protocols, authentication systems, or message formats.
- Connection closure depends on the underlying Unreal Engine WebSockets implementation delivering its close event.

## Contributing

Contributions, bug reports, and suggestions are welcome!

If you encounter a problem or have an idea for an improvement, please open an issue in the repository. Pull requests are also welcome.

## Support

- **Issues:** [GitHub Issues](https://github.com/ArvurRobin/WebSocketsSubsystem/issues)
- **Repository:** [WebSocketsSubsystem](https://github.com/ArvurRobin/WebSocketsSubsystem)

---

Developed by [Robin Hasenbach](https://github.com/ArvurRobin).
