/**
 * @file KGKNet.hpp
 * @brief Networking module for KillerGK
 * 
 * Provides HTTP client, WebSocket, TCP/UDP sockets, and file download functionality.
 */

#pragma once

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <vector>
#include <cstdint>
#include <atomic>
#include <thread>
#include <mutex>

namespace KGKNet {

// Forward declarations
class HttpClientImpl;
class WebSocketImpl;
class TcpSocketImpl;
class UdpSocketImpl;
class FileDownloaderImpl;

/**
 * @enum HttpMethod
 * @brief HTTP request methods
 */
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS
};

/**
 * @struct HttpRequest
 * @brief HTTP request data
 */
struct HttpRequest {
    HttpMethod method = HttpMethod::GET;
    std::string url;
    std::map<std::string, std::string> headers;
    std::string body;
    int timeoutMs = 30000;
};

/**
 * @struct HttpResponse
 * @brief HTTP response data
 */
struct HttpResponse {
    int statusCode = 0;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string error;

    [[nodiscard]] bool isSuccess() const { return statusCode >= 200 && statusCode < 300; }
    [[nodiscard]] bool isRedirect() const { return statusCode >= 300 && statusCode < 400; }
    [[nodiscard]] bool isClientError() const { return statusCode >= 400 && statusCode < 500; }
    [[nodiscard]] bool isServerError() const { return statusCode >= 500; }
};


/**
 * @class HttpClient
 * @brief HTTP client for making requests
 * 
 * Supports GET, POST, PUT, DELETE methods with headers and body.
 * Uses WinHTTP on Windows and libcurl on Unix platforms.
 */
class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    // Synchronous request methods
    HttpResponse get(const std::string& url);
    HttpResponse post(const std::string& url, const std::string& body);
    HttpResponse put(const std::string& url, const std::string& body);
    HttpResponse del(const std::string& url);
    HttpResponse patch(const std::string& url, const std::string& body);
    HttpResponse head(const std::string& url);
    HttpResponse options(const std::string& url);

    // Generic request method
    HttpResponse request(const HttpRequest& request);

    // Configuration (Builder Pattern)
    HttpClient& header(const std::string& key, const std::string& value);
    HttpClient& headers(const std::map<std::string, std::string>& headers);
    HttpClient& timeout(int milliseconds);
    HttpClient& contentType(const std::string& type);
    HttpClient& authorization(const std::string& token);
    HttpClient& basicAuth(const std::string& username, const std::string& password);
    HttpClient& userAgent(const std::string& agent);
    HttpClient& followRedirects(bool follow);

    // Clear configuration
    void clearHeaders();
    void reset();

    // Async versions
    void getAsync(const std::string& url, std::function<void(HttpResponse)> callback);
    void postAsync(const std::string& url, const std::string& body, 
                   std::function<void(HttpResponse)> callback);
    void putAsync(const std::string& url, const std::string& body,
                  std::function<void(HttpResponse)> callback);
    void delAsync(const std::string& url, std::function<void(HttpResponse)> callback);
    void requestAsync(const HttpRequest& request, std::function<void(HttpResponse)> callback);

private:
    std::unique_ptr<HttpClientImpl> m_impl;
};

/**
 * @brief Handle to a WebSocket connection
 */
using WebSocketHandle = std::shared_ptr<WebSocketImpl>;

/**
 * @enum WebSocketState
 * @brief WebSocket connection state
 */
enum class WebSocketState {
    Connecting,
    Open,
    Closing,
    Closed
};

/**
 * @class WebSocket
 * @brief WebSocket client for bidirectional communication
 * 
 * Supports text and binary messages with auto-reconnection.
 */
class WebSocket {
public:
    static WebSocket connect(const std::string& url);

    // Events
    WebSocket& onOpen(std::function<void()> callback);
    WebSocket& onMessage(std::function<void(const std::string&)> callback);
    WebSocket& onBinaryMessage(std::function<void(const std::vector<uint8_t>&)> callback);
    WebSocket& onClose(std::function<void(int, const std::string&)> callback);
    WebSocket& onError(std::function<void(const std::string&)> callback);

    // Operations
    void send(const std::string& message);
    void send(const std::vector<uint8_t>& data);
    void ping();
    void close(int code = 1000, const std::string& reason = "");

    // Configuration
    WebSocket& autoReconnect(bool enabled);
    WebSocket& reconnectDelay(int milliseconds);
    WebSocket& maxReconnectAttempts(int attempts);
    WebSocket& subprotocol(const std::string& protocol);
    WebSocket& header(const std::string& key, const std::string& value);

    // State
    [[nodiscard]] WebSocketState state() const;
    [[nodiscard]] bool isConnected() const;

    WebSocketHandle build();

private:
    WebSocket();
    std::shared_ptr<WebSocketImpl> m_impl;
};


/**
 * @brief Handle to a TCP socket
 */
using TcpSocketHandle = std::shared_ptr<TcpSocketImpl>;

/**
 * @brief Handle to a UDP socket
 */
using UdpSocketHandle = std::shared_ptr<UdpSocketImpl>;

/**
 * @enum SocketState
 * @brief Socket connection state
 */
enum class SocketState {
    Disconnected,
    Connecting,
    Connected,
    Listening,
    Error
};

/**
 * @struct SocketAddress
 * @brief Network address for sockets
 */
struct SocketAddress {
    std::string host;
    uint16_t port = 0;

    SocketAddress() = default;
    SocketAddress(const std::string& h, uint16_t p) : host(h), port(p) {}
};

/**
 * @class TcpSocket
 * @brief TCP socket for reliable stream communication
 */
class TcpSocket {
public:
    static TcpSocket create();

    // Connection
    bool connect(const std::string& host, uint16_t port);
    bool connect(const SocketAddress& address);
    void connectAsync(const std::string& host, uint16_t port, 
                      std::function<void(bool)> callback);
    void disconnect();

    // Server operations
    bool listen(uint16_t port, int backlog = 10);
    TcpSocketHandle accept();
    void acceptAsync(std::function<void(TcpSocketHandle)> callback);

    // Data transfer
    int send(const void* data, size_t size);
    int send(const std::string& data);
    int send(const std::vector<uint8_t>& data);
    int receive(void* buffer, size_t size);
    std::string receiveString(size_t maxSize = 4096);
    std::vector<uint8_t> receiveBytes(size_t maxSize = 4096);

    // Async data transfer
    void sendAsync(const std::vector<uint8_t>& data, 
                   std::function<void(int)> callback);
    void receiveAsync(size_t maxSize, 
                      std::function<void(std::vector<uint8_t>)> callback);

    // Events
    TcpSocket& onData(std::function<void(const std::vector<uint8_t>&)> callback);
    TcpSocket& onDisconnect(std::function<void()> callback);
    TcpSocket& onError(std::function<void(const std::string&)> callback);

    // Configuration
    TcpSocket& noDelay(bool enabled);
    TcpSocket& keepAlive(bool enabled);
    TcpSocket& receiveTimeout(int milliseconds);
    TcpSocket& sendTimeout(int milliseconds);
    TcpSocket& receiveBufferSize(int size);
    TcpSocket& sendBufferSize(int size);

    // State
    [[nodiscard]] SocketState state() const;
    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] SocketAddress localAddress() const;
    [[nodiscard]] SocketAddress remoteAddress() const;

    TcpSocketHandle build();

private:
    TcpSocket();
    std::shared_ptr<TcpSocketImpl> m_impl;
};

/**
 * @class UdpSocket
 * @brief UDP socket for datagram communication
 */
class UdpSocket {
public:
    static UdpSocket create();

    // Binding
    bool bind(uint16_t port);
    bool bind(const std::string& host, uint16_t port);

    // Data transfer
    int sendTo(const void* data, size_t size, const SocketAddress& address);
    int sendTo(const std::string& data, const SocketAddress& address);
    int sendTo(const std::vector<uint8_t>& data, const SocketAddress& address);
    int receiveFrom(void* buffer, size_t size, SocketAddress& sender);
    std::pair<std::vector<uint8_t>, SocketAddress> receiveFrom(size_t maxSize = 65535);

    // Async data transfer
    void sendToAsync(const std::vector<uint8_t>& data, const SocketAddress& address,
                     std::function<void(int)> callback);
    void receiveFromAsync(size_t maxSize,
                          std::function<void(std::vector<uint8_t>, SocketAddress)> callback);

    // Events
    UdpSocket& onData(std::function<void(const std::vector<uint8_t>&, const SocketAddress&)> callback);
    UdpSocket& onError(std::function<void(const std::string&)> callback);

    // Configuration
    UdpSocket& broadcast(bool enabled);
    UdpSocket& multicastJoin(const std::string& group);
    UdpSocket& multicastLeave(const std::string& group);
    UdpSocket& multicastTTL(int ttl);
    UdpSocket& receiveBufferSize(int size);
    UdpSocket& sendBufferSize(int size);

    // State
    [[nodiscard]] bool isBound() const;
    [[nodiscard]] SocketAddress localAddress() const;

    UdpSocketHandle build();

private:
    UdpSocket();
    std::shared_ptr<UdpSocketImpl> m_impl;
};


/**
 * @enum DownloadState
 * @brief Download state
 */
enum class DownloadState {
    Idle,
    Downloading,
    Paused,
    Completed,
    Failed,
    Cancelled
};

/**
 * @struct DownloadProgress
 * @brief Download progress information
 */
struct DownloadProgress {
    size_t bytesDownloaded = 0;
    size_t totalBytes = 0;
    float percentage = 0.0f;
    float speedBytesPerSec = 0.0f;
    int estimatedSecondsRemaining = -1;
};

/**
 * @struct DownloadInfo
 * @brief Download information
 */
struct DownloadInfo {
    std::string url;
    std::string destination;
    DownloadState state = DownloadState::Idle;
    DownloadProgress progress;
    std::string error;
};

/**
 * @class FileDownloader
 * @brief File download manager with pause/resume support
 */
class FileDownloader {
public:
    FileDownloader();
    ~FileDownloader();

    /**
     * @brief Download a file
     * @param url URL to download from
     * @param destination Local file path
     * @return true if download started successfully
     */
    bool download(const std::string& url, const std::string& destination);

    // Events
    FileDownloader& onProgress(std::function<void(DownloadProgress)> callback);
    FileDownloader& onComplete(std::function<void(bool, const std::string&)> callback);
    FileDownloader& onStateChange(std::function<void(DownloadState)> callback);

    // Controls
    void pause();
    void resume();
    void cancel();

    // Configuration
    FileDownloader& timeout(int milliseconds);
    FileDownloader& header(const std::string& key, const std::string& value);
    FileDownloader& maxRetries(int retries);
    FileDownloader& chunkSize(size_t size);

    // State
    [[nodiscard]] DownloadState state() const;
    [[nodiscard]] DownloadProgress progress() const;
    [[nodiscard]] DownloadInfo info() const;

private:
    std::unique_ptr<FileDownloaderImpl> m_impl;
};

/**
 * @brief Initialize the networking module
 * @return true if initialization succeeded
 */
bool initialize();

/**
 * @brief Shutdown the networking module
 */
void shutdown();

/**
 * @brief URL encode a string
 */
std::string urlEncode(const std::string& str);

/**
 * @brief URL decode a string
 */
std::string urlDecode(const std::string& str);

/**
 * @brief Base64 encode data
 */
std::string base64Encode(const std::vector<uint8_t>& data);
std::string base64Encode(const std::string& str);

/**
 * @brief Base64 decode data
 */
std::vector<uint8_t> base64Decode(const std::string& encoded);

} // namespace KGKNet
