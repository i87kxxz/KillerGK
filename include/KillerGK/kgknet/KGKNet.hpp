/**
 * @file KGKNet.hpp
 * @brief Networking module for KillerGK
 */

#pragma once

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <vector>

namespace KGKNet {

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
};

/**
 * @class HttpClient
 * @brief HTTP client for making requests
 */
class HttpClient {
public:
    HttpClient();

    // Request methods
    HttpResponse get(const std::string& url);
    HttpResponse post(const std::string& url, const std::string& body);
    HttpResponse put(const std::string& url, const std::string& body);
    HttpResponse del(const std::string& url);

    // Configuration
    HttpClient& header(const std::string& key, const std::string& value);
    HttpClient& timeout(int milliseconds);
    HttpClient& contentType(const std::string& type);

    // Async versions
    void getAsync(const std::string& url, std::function<void(HttpResponse)> callback);
    void postAsync(const std::string& url, const std::string& body, 
                   std::function<void(HttpResponse)> callback);

private:
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @brief Handle to a WebSocket connection
 */
using WebSocketHandle = std::shared_ptr<class WebSocketImpl>;

/**
 * @class WebSocket
 * @brief WebSocket client
 */
class WebSocket {
public:
    static WebSocket connect(const std::string& url);

    // Events
    WebSocket& onOpen(std::function<void()> callback);
    WebSocket& onMessage(std::function<void(const std::string&)> callback);
    WebSocket& onClose(std::function<void(int, const std::string&)> callback);
    WebSocket& onError(std::function<void(const std::string&)> callback);

    // Operations
    void send(const std::string& message);
    void send(const std::vector<uint8_t>& data);
    void close();

    // Configuration
    WebSocket& autoReconnect(bool enabled);
    WebSocket& reconnectDelay(int milliseconds);

    WebSocketHandle build();

private:
    WebSocket();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @struct DownloadProgress
 * @brief Download progress information
 */
struct DownloadProgress {
    size_t bytesDownloaded = 0;
    size_t totalBytes = 0;
    float percentage = 0.0f;
};

/**
 * @class FileDownloader
 * @brief File download manager
 */
class FileDownloader {
public:
    FileDownloader();

    /**
     * @brief Download a file
     * @param url URL to download from
     * @param destination Local file path
     */
    void download(const std::string& url, const std::string& destination);

    // Events
    FileDownloader& onProgress(std::function<void(DownloadProgress)> callback);
    FileDownloader& onComplete(std::function<void(bool, const std::string&)> callback);

    // Controls
    void pause();
    void resume();
    void cancel();

private:
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

} // namespace KGKNet
