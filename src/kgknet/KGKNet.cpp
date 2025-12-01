/**
 * @file KGKNet.cpp
 * @brief Networking module implementation
 * 
 * Platform-specific implementations:
 * - Windows: WinHTTP for HTTP, Winsock2 for sockets
 * - Unix: libcurl for HTTP (if available), BSD sockets
 */

#include "KillerGK/kgknet/KGKNet.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <queue>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
    #include <winhttp.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #pragma comment(lib, "winhttp.lib")
    
    using SocketType = SOCKET;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_ERROR_VALUE SOCKET_ERROR
    #define CloseSocketFunc closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    
    using SocketType = int;
    #define INVALID_SOCKET_VALUE (-1)
    #define SOCKET_ERROR_VALUE (-1)
    #define CloseSocketFunc close
    
    #ifdef KGK_HAS_CURL
        #include <curl/curl.h>
    #endif
#endif

namespace KGKNet {

// =============================================================================
// Global State
// =============================================================================

static std::atomic<bool> g_initialized{false};

bool initialize() {
    if (g_initialized.exchange(true)) {
        return true; // Already initialized
    }

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        g_initialized = false;
        return false;
    }
#endif

#if !defined(_WIN32) && defined(KGK_HAS_CURL)
    curl_global_init(CURL_GLOBAL_ALL);
#endif

    return true;
}

void shutdown() {
    if (!g_initialized.exchange(false)) {
        return; // Not initialized
    }

#ifdef _WIN32
    WSACleanup();
#endif

#if !defined(_WIN32) && defined(KGK_HAS_CURL)
    curl_global_cleanup();
#endif
}


// =============================================================================
// Utility Functions
// =============================================================================

std::string urlEncode(const std::string& str) {
    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex;

    for (char c : str) {
        if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            encoded << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
        }
    }

    return encoded.str();
}

std::string urlDecode(const std::string& str) {
    std::string decoded;
    decoded.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%' && i + 2 < str.size()) {
            int value;
            std::istringstream iss(str.substr(i + 1, 2));
            if (iss >> std::hex >> value) {
                decoded += static_cast<char>(value);
                i += 2;
            } else {
                decoded += str[i];
            }
        } else if (str[i] == '+') {
            decoded += ' ';
        } else {
            decoded += str[i];
        }
    }

    return decoded;
}

static const char* base64Chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64Encode(const std::vector<uint8_t>& data) {
    std::string result;
    result.reserve(((data.size() + 2) / 3) * 4);

    for (size_t i = 0; i < data.size(); i += 3) {
        uint32_t n = static_cast<uint32_t>(data[i]) << 16;
        if (i + 1 < data.size()) n |= static_cast<uint32_t>(data[i + 1]) << 8;
        if (i + 2 < data.size()) n |= static_cast<uint32_t>(data[i + 2]);

        result += base64Chars[(n >> 18) & 0x3F];
        result += base64Chars[(n >> 12) & 0x3F];
        result += (i + 1 < data.size()) ? base64Chars[(n >> 6) & 0x3F] : '=';
        result += (i + 2 < data.size()) ? base64Chars[n & 0x3F] : '=';
    }

    return result;
}

std::string base64Encode(const std::string& str) {
    return base64Encode(std::vector<uint8_t>(str.begin(), str.end()));
}

std::vector<uint8_t> base64Decode(const std::string& encoded) {
    std::vector<uint8_t> result;
    result.reserve((encoded.size() / 4) * 3);

    auto indexOf = [](char c) -> int {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        if (c >= '0' && c <= '9') return c - '0' + 52;
        if (c == '+') return 62;
        if (c == '/') return 63;
        return -1;
    };

    for (size_t i = 0; i < encoded.size(); i += 4) {
        int n0 = indexOf(encoded[i]);
        int n1 = (i + 1 < encoded.size()) ? indexOf(encoded[i + 1]) : 0;
        int n2 = (i + 2 < encoded.size()) ? indexOf(encoded[i + 2]) : 0;
        int n3 = (i + 3 < encoded.size()) ? indexOf(encoded[i + 3]) : 0;

        if (n0 < 0 || n1 < 0) break;

        result.push_back(static_cast<uint8_t>((n0 << 2) | (n1 >> 4)));
        if (encoded[i + 2] != '=' && n2 >= 0) {
            result.push_back(static_cast<uint8_t>(((n1 & 0x0F) << 4) | (n2 >> 2)));
        }
        if (encoded[i + 3] != '=' && n3 >= 0) {
            result.push_back(static_cast<uint8_t>(((n2 & 0x03) << 6) | n3));
        }
    }

    return result;
}


// =============================================================================
// HttpClient Implementation
// =============================================================================

class HttpClientImpl {
public:
    std::map<std::string, std::string> headers;
    int timeoutMs = 30000;
    std::string contentType = "application/json";
    std::string userAgent = "KGKNet/1.0";
    bool followRedirects = true;

#ifdef _WIN32
    HINTERNET hSession = nullptr;

    HttpClientImpl() {
        initialize();
        hSession = WinHttpOpen(
            L"KGKNet/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0
        );
    }

    ~HttpClientImpl() {
        if (hSession) {
            WinHttpCloseHandle(hSession);
        }
    }

    HttpResponse performRequest(const HttpRequest& request) {
        HttpResponse response;

        if (!hSession) {
            response.error = "Failed to initialize HTTP session";
            return response;
        }

        // Parse URL
        URL_COMPONENTS urlComp = {};
        urlComp.dwStructSize = sizeof(urlComp);
        urlComp.dwSchemeLength = -1;
        urlComp.dwHostNameLength = -1;
        urlComp.dwUrlPathLength = -1;
        urlComp.dwExtraInfoLength = -1;

        std::wstring wUrl(request.url.begin(), request.url.end());
        if (!WinHttpCrackUrl(wUrl.c_str(), 0, 0, &urlComp)) {
            response.error = "Invalid URL";
            return response;
        }

        std::wstring hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);
        std::wstring urlPath(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
        if (urlComp.dwExtraInfoLength > 0) {
            urlPath += std::wstring(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);
        }

        bool isHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);
        INTERNET_PORT port = urlComp.nPort ? urlComp.nPort : (isHttps ? 443 : 80);

        // Connect
        HINTERNET hConnect = WinHttpConnect(hSession, hostName.c_str(), port, 0);
        if (!hConnect) {
            response.error = "Failed to connect to server";
            return response;
        }

        // Determine method
        std::wstring method;
        switch (request.method) {
            case HttpMethod::GET: method = L"GET"; break;
            case HttpMethod::POST: method = L"POST"; break;
            case HttpMethod::PUT: method = L"PUT"; break;
            case HttpMethod::DELETE: method = L"DELETE"; break;
            case HttpMethod::PATCH: method = L"PATCH"; break;
            case HttpMethod::HEAD: method = L"HEAD"; break;
            case HttpMethod::OPTIONS: method = L"OPTIONS"; break;
        }

        // Open request
        DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(
            hConnect, method.c_str(), urlPath.c_str(),
            nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags
        );

        if (!hRequest) {
            WinHttpCloseHandle(hConnect);
            response.error = "Failed to create request";
            return response;
        }

        // Set timeout
        WinHttpSetTimeouts(hRequest, request.timeoutMs, request.timeoutMs, 
                          request.timeoutMs, request.timeoutMs);

        // Add headers
        std::wstring headerStr;
        for (const auto& [key, value] : headers) {
            headerStr += std::wstring(key.begin(), key.end()) + L": " + 
                        std::wstring(value.begin(), value.end()) + L"\r\n";
        }
        for (const auto& [key, value] : request.headers) {
            headerStr += std::wstring(key.begin(), key.end()) + L": " + 
                        std::wstring(value.begin(), value.end()) + L"\r\n";
        }
        if (!headerStr.empty()) {
            WinHttpAddRequestHeaders(hRequest, headerStr.c_str(), -1, 
                                    WINHTTP_ADDREQ_FLAG_ADD);
        }

        // Send request
        BOOL result = WinHttpSendRequest(
            hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            request.body.empty() ? WINHTTP_NO_REQUEST_DATA : 
                const_cast<char*>(request.body.c_str()),
            static_cast<DWORD>(request.body.size()),
            static_cast<DWORD>(request.body.size()),
            0
        );

        if (!result) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            response.error = "Failed to send request";
            return response;
        }

        // Receive response
        result = WinHttpReceiveResponse(hRequest, nullptr);
        if (!result) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            response.error = "Failed to receive response";
            return response;
        }

        // Get status code
        DWORD statusCode = 0;
        DWORD size = sizeof(statusCode);
        WinHttpQueryHeaders(hRequest, 
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size, WINHTTP_NO_HEADER_INDEX);
        response.statusCode = static_cast<int>(statusCode);

        // Read response body
        std::string body;
        DWORD bytesAvailable = 0;
        do {
            bytesAvailable = 0;
            WinHttpQueryDataAvailable(hRequest, &bytesAvailable);
            if (bytesAvailable > 0) {
                std::vector<char> buffer(bytesAvailable + 1);
                DWORD bytesRead = 0;
                if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead)) {
                    body.append(buffer.data(), bytesRead);
                }
            }
        } while (bytesAvailable > 0);

        response.body = body;

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);

        return response;
    }
#else
    // Unix implementation using sockets or libcurl
    HttpResponse performRequest(const HttpRequest& request) {
        HttpResponse response;

#ifdef KGK_HAS_CURL
        CURL* curl = curl_easy_init();
        if (!curl) {
            response.error = "Failed to initialize CURL";
            return response;
        }

        // Response data
        std::string responseBody;
        std::string responseHeaders;

        auto writeCallback = [](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
            auto* str = static_cast<std::string*>(userdata);
            str->append(ptr, size * nmemb);
            return size * nmemb;
        };

        curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &responseHeaders);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, request.timeoutMs);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, followRedirects ? 1L : 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());

        // Set method
        switch (request.method) {
            case HttpMethod::POST:
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body.c_str());
                break;
            case HttpMethod::PUT:
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body.c_str());
                break;
            case HttpMethod::DELETE:
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
                break;
            case HttpMethod::PATCH:
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body.c_str());
                break;
            case HttpMethod::HEAD:
                curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
                break;
            case HttpMethod::OPTIONS:
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "OPTIONS");
                break;
            default:
                break;
        }

        // Set headers
        struct curl_slist* headerList = nullptr;
        for (const auto& [key, value] : headers) {
            std::string header = key + ": " + value;
            headerList = curl_slist_append(headerList, header.c_str());
        }
        for (const auto& [key, value] : request.headers) {
            std::string header = key + ": " + value;
            headerList = curl_slist_append(headerList, header.c_str());
        }
        if (headerList) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
        }

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            response.error = curl_easy_strerror(res);
        } else {
            long httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            response.statusCode = static_cast<int>(httpCode);
            response.body = responseBody;
        }

        if (headerList) {
            curl_slist_free_all(headerList);
        }
        curl_easy_cleanup(curl);
#else
        // Basic socket-based HTTP implementation
        response.error = "HTTP client requires libcurl on this platform";
        response.statusCode = 501;
#endif

        return response;
    }
#endif
};


HttpClient::HttpClient() : m_impl(std::make_unique<HttpClientImpl>()) {}
HttpClient::~HttpClient() = default;

HttpResponse HttpClient::get(const std::string& url) {
    HttpRequest req;
    req.method = HttpMethod::GET;
    req.url = url;
    req.timeoutMs = m_impl->timeoutMs;
    req.headers = m_impl->headers;
    return m_impl->performRequest(req);
}

HttpResponse HttpClient::post(const std::string& url, const std::string& body) {
    HttpRequest req;
    req.method = HttpMethod::POST;
    req.url = url;
    req.body = body;
    req.timeoutMs = m_impl->timeoutMs;
    req.headers = m_impl->headers;
    if (!m_impl->contentType.empty()) {
        req.headers["Content-Type"] = m_impl->contentType;
    }
    return m_impl->performRequest(req);
}

HttpResponse HttpClient::put(const std::string& url, const std::string& body) {
    HttpRequest req;
    req.method = HttpMethod::PUT;
    req.url = url;
    req.body = body;
    req.timeoutMs = m_impl->timeoutMs;
    req.headers = m_impl->headers;
    if (!m_impl->contentType.empty()) {
        req.headers["Content-Type"] = m_impl->contentType;
    }
    return m_impl->performRequest(req);
}

HttpResponse HttpClient::del(const std::string& url) {
    HttpRequest req;
    req.method = HttpMethod::DELETE;
    req.url = url;
    req.timeoutMs = m_impl->timeoutMs;
    req.headers = m_impl->headers;
    return m_impl->performRequest(req);
}

HttpResponse HttpClient::patch(const std::string& url, const std::string& body) {
    HttpRequest req;
    req.method = HttpMethod::PATCH;
    req.url = url;
    req.body = body;
    req.timeoutMs = m_impl->timeoutMs;
    req.headers = m_impl->headers;
    if (!m_impl->contentType.empty()) {
        req.headers["Content-Type"] = m_impl->contentType;
    }
    return m_impl->performRequest(req);
}

HttpResponse HttpClient::head(const std::string& url) {
    HttpRequest req;
    req.method = HttpMethod::HEAD;
    req.url = url;
    req.timeoutMs = m_impl->timeoutMs;
    req.headers = m_impl->headers;
    return m_impl->performRequest(req);
}

HttpResponse HttpClient::options(const std::string& url) {
    HttpRequest req;
    req.method = HttpMethod::OPTIONS;
    req.url = url;
    req.timeoutMs = m_impl->timeoutMs;
    req.headers = m_impl->headers;
    return m_impl->performRequest(req);
}

HttpResponse HttpClient::request(const HttpRequest& request) {
    HttpRequest req = request;
    for (const auto& [key, value] : m_impl->headers) {
        if (req.headers.find(key) == req.headers.end()) {
            req.headers[key] = value;
        }
    }
    if (req.timeoutMs == 0) {
        req.timeoutMs = m_impl->timeoutMs;
    }
    return m_impl->performRequest(req);
}

HttpClient& HttpClient::header(const std::string& key, const std::string& value) {
    m_impl->headers[key] = value;
    return *this;
}

HttpClient& HttpClient::headers(const std::map<std::string, std::string>& headers) {
    for (const auto& [key, value] : headers) {
        m_impl->headers[key] = value;
    }
    return *this;
}

HttpClient& HttpClient::timeout(int milliseconds) {
    m_impl->timeoutMs = milliseconds;
    return *this;
}

HttpClient& HttpClient::contentType(const std::string& type) {
    m_impl->contentType = type;
    m_impl->headers["Content-Type"] = type;
    return *this;
}

HttpClient& HttpClient::authorization(const std::string& token) {
    m_impl->headers["Authorization"] = token;
    return *this;
}

HttpClient& HttpClient::basicAuth(const std::string& username, const std::string& password) {
    std::string credentials = username + ":" + password;
    std::string encoded = base64Encode(credentials);
    m_impl->headers["Authorization"] = "Basic " + encoded;
    return *this;
}

HttpClient& HttpClient::userAgent(const std::string& agent) {
    m_impl->userAgent = agent;
    m_impl->headers["User-Agent"] = agent;
    return *this;
}

HttpClient& HttpClient::followRedirects(bool follow) {
    m_impl->followRedirects = follow;
    return *this;
}

void HttpClient::clearHeaders() {
    m_impl->headers.clear();
}

void HttpClient::reset() {
    m_impl->headers.clear();
    m_impl->timeoutMs = 30000;
    m_impl->contentType = "application/json";
    m_impl->userAgent = "KGKNet/1.0";
    m_impl->followRedirects = true;
}

void HttpClient::getAsync(const std::string& url, std::function<void(HttpResponse)> callback) {
    std::thread([this, url, callback]() {
        auto response = get(url);
        if (callback) callback(response);
    }).detach();
}

void HttpClient::postAsync(const std::string& url, const std::string& body,
                           std::function<void(HttpResponse)> callback) {
    std::thread([this, url, body, callback]() {
        auto response = post(url, body);
        if (callback) callback(response);
    }).detach();
}

void HttpClient::putAsync(const std::string& url, const std::string& body,
                          std::function<void(HttpResponse)> callback) {
    std::thread([this, url, body, callback]() {
        auto response = put(url, body);
        if (callback) callback(response);
    }).detach();
}

void HttpClient::delAsync(const std::string& url, std::function<void(HttpResponse)> callback) {
    std::thread([this, url, callback]() {
        auto response = del(url);
        if (callback) callback(response);
    }).detach();
}

void HttpClient::requestAsync(const HttpRequest& request, 
                              std::function<void(HttpResponse)> callback) {
    std::thread([this, request, callback]() {
        auto response = this->request(request);
        if (callback) callback(response);
    }).detach();
}


// =============================================================================
// WebSocket Implementation
// =============================================================================

class WebSocketImpl : public std::enable_shared_from_this<WebSocketImpl> {
public:
    std::string url;
    std::map<std::string, std::string> headers;
    std::string subprotocol;
    
    std::function<void()> onOpenCallback;
    std::function<void(const std::string&)> onMessageCallback;
    std::function<void(const std::vector<uint8_t>&)> onBinaryMessageCallback;
    std::function<void(int, const std::string&)> onCloseCallback;
    std::function<void(const std::string&)> onErrorCallback;
    
    bool autoReconnectEnabled = true;
    int reconnectDelayMs = 5000;
    int maxReconnectAttempts = 10;
    int reconnectAttempts = 0;
    
    WebSocketState currentState = WebSocketState::Closed;
    std::atomic<bool> running{false};
    std::thread receiveThread;
    std::mutex sendMutex;
    
    SocketType socket = INVALID_SOCKET_VALUE;

    ~WebSocketImpl() {
        close(1000, "");
    }

    bool connect() {
        initialize();
        currentState = WebSocketState::Connecting;

        // Parse URL (ws://host:port/path or wss://host:port/path)
        std::string host;
        uint16_t port = 80;
        std::string path = "/";
        bool secure = false;

        std::string urlCopy = url;
        if (urlCopy.substr(0, 5) == "wss://") {
            secure = true;
            port = 443;
            urlCopy = urlCopy.substr(6);
        } else if (urlCopy.substr(0, 5) == "ws://") {
            urlCopy = urlCopy.substr(5);
        }

        size_t pathPos = urlCopy.find('/');
        if (pathPos != std::string::npos) {
            path = urlCopy.substr(pathPos);
            urlCopy = urlCopy.substr(0, pathPos);
        }

        size_t portPos = urlCopy.find(':');
        if (portPos != std::string::npos) {
            host = urlCopy.substr(0, portPos);
            port = static_cast<uint16_t>(std::stoi(urlCopy.substr(portPos + 1)));
        } else {
            host = urlCopy;
        }

        // Create socket
        socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket == INVALID_SOCKET_VALUE) {
            currentState = WebSocketState::Closed;
            if (onErrorCallback) onErrorCallback("Failed to create socket");
            return false;
        }

        // Resolve host
        struct addrinfo hints = {}, *result = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0) {
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
            currentState = WebSocketState::Closed;
            if (onErrorCallback) onErrorCallback("Failed to resolve host");
            return false;
        }

        // Connect
        if (::connect(socket, result->ai_addr, static_cast<int>(result->ai_addrlen)) != 0) {
            freeaddrinfo(result);
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
            currentState = WebSocketState::Closed;
            if (onErrorCallback) onErrorCallback("Failed to connect");
            return false;
        }
        freeaddrinfo(result);

        // WebSocket handshake
        std::string key = base64Encode(std::vector<uint8_t>(16, 0)); // Should be random
        std::ostringstream request;
        request << "GET " << path << " HTTP/1.1\r\n";
        request << "Host: " << host << ":" << port << "\r\n";
        request << "Upgrade: websocket\r\n";
        request << "Connection: Upgrade\r\n";
        request << "Sec-WebSocket-Key: " << key << "\r\n";
        request << "Sec-WebSocket-Version: 13\r\n";
        if (!subprotocol.empty()) {
            request << "Sec-WebSocket-Protocol: " << subprotocol << "\r\n";
        }
        for (const auto& [k, v] : headers) {
            request << k << ": " << v << "\r\n";
        }
        request << "\r\n";

        std::string requestStr = request.str();
        if (::send(socket, requestStr.c_str(), static_cast<int>(requestStr.size()), 0) <= 0) {
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
            currentState = WebSocketState::Closed;
            if (onErrorCallback) onErrorCallback("Failed to send handshake");
            return false;
        }

        // Read response
        char buffer[1024];
        int received = ::recv(socket, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
            currentState = WebSocketState::Closed;
            if (onErrorCallback) onErrorCallback("Failed to receive handshake response");
            return false;
        }
        buffer[received] = '\0';

        std::string response(buffer);
        if (response.find("101") == std::string::npos) {
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
            currentState = WebSocketState::Closed;
            if (onErrorCallback) onErrorCallback("WebSocket handshake failed");
            return false;
        }

        currentState = WebSocketState::Open;
        running = true;
        reconnectAttempts = 0;

        if (onOpenCallback) onOpenCallback();

        // Start receive thread
        receiveThread = std::thread([this]() { receiveLoop(); });

        return true;
    }

    void receiveLoop() {
        std::vector<uint8_t> buffer(65536);
        
        while (running && socket != INVALID_SOCKET_VALUE) {
            int received = ::recv(socket, reinterpret_cast<char*>(buffer.data()), 
                                 static_cast<int>(buffer.size()), 0);
            
            if (received <= 0) {
                if (running) {
                    handleDisconnect();
                }
                break;
            }

            // Parse WebSocket frame (simplified)
            if (received >= 2) {
                uint8_t opcode = buffer[0] & 0x0F;
                bool masked = (buffer[1] & 0x80) != 0;
                size_t payloadLen = buffer[1] & 0x7F;
                size_t headerLen = 2;

                if (payloadLen == 126 && received >= 4) {
                    payloadLen = (static_cast<size_t>(buffer[2]) << 8) | buffer[3];
                    headerLen = 4;
                } else if (payloadLen == 127 && received >= 10) {
                    payloadLen = 0;
                    for (int i = 0; i < 8; ++i) {
                        payloadLen = (payloadLen << 8) | buffer[2 + i];
                    }
                    headerLen = 10;
                }

                if (masked) headerLen += 4;

                if (headerLen + payloadLen <= static_cast<size_t>(received)) {
                    std::vector<uint8_t> payload(buffer.begin() + headerLen, 
                                                  buffer.begin() + headerLen + payloadLen);

                    if (masked) {
                        uint8_t mask[4];
                        std::copy(buffer.begin() + headerLen - 4, 
                                 buffer.begin() + headerLen, mask);
                        for (size_t i = 0; i < payload.size(); ++i) {
                            payload[i] ^= mask[i % 4];
                        }
                    }

                    switch (opcode) {
                        case 0x01: // Text
                            if (onMessageCallback) {
                                onMessageCallback(std::string(payload.begin(), payload.end()));
                            }
                            break;
                        case 0x02: // Binary
                            if (onBinaryMessageCallback) {
                                onBinaryMessageCallback(payload);
                            }
                            break;
                        case 0x08: // Close
                            handleDisconnect();
                            return;
                        case 0x09: // Ping
                            sendPong(payload);
                            break;
                        case 0x0A: // Pong
                            break;
                    }
                }
            }
        }
    }

    void handleDisconnect() {
        currentState = WebSocketState::Closed;
        
        if (socket != INVALID_SOCKET_VALUE) {
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
        }

        if (onCloseCallback) {
            onCloseCallback(1006, "Connection closed");
        }

        // Auto-reconnect
        if (autoReconnectEnabled && reconnectAttempts < maxReconnectAttempts) {
            ++reconnectAttempts;
            std::this_thread::sleep_for(std::chrono::milliseconds(reconnectDelayMs));
            if (running) {
                connect();
            }
        }
    }

    void send(const std::string& message) {
        sendFrame(0x01, std::vector<uint8_t>(message.begin(), message.end()));
    }

    void send(const std::vector<uint8_t>& data) {
        sendFrame(0x02, data);
    }

    void sendPong(const std::vector<uint8_t>& data) {
        sendFrame(0x0A, data);
    }

    void sendFrame(uint8_t opcode, const std::vector<uint8_t>& payload) {
        if (socket == INVALID_SOCKET_VALUE || currentState != WebSocketState::Open) {
            return;
        }

        std::lock_guard<std::mutex> lock(sendMutex);

        std::vector<uint8_t> frame;
        frame.push_back(0x80 | opcode); // FIN + opcode

        // Payload length
        if (payload.size() < 126) {
            frame.push_back(0x80 | static_cast<uint8_t>(payload.size())); // Masked
        } else if (payload.size() < 65536) {
            frame.push_back(0x80 | 126);
            frame.push_back(static_cast<uint8_t>((payload.size() >> 8) & 0xFF));
            frame.push_back(static_cast<uint8_t>(payload.size() & 0xFF));
        } else {
            frame.push_back(0x80 | 127);
            for (int i = 7; i >= 0; --i) {
                frame.push_back(static_cast<uint8_t>((payload.size() >> (i * 8)) & 0xFF));
            }
        }

        // Mask key (should be random)
        uint8_t mask[4] = {0x12, 0x34, 0x56, 0x78};
        frame.insert(frame.end(), mask, mask + 4);

        // Masked payload
        for (size_t i = 0; i < payload.size(); ++i) {
            frame.push_back(payload[i] ^ mask[i % 4]);
        }

        ::send(socket, reinterpret_cast<const char*>(frame.data()), 
               static_cast<int>(frame.size()), 0);
    }

    void close(int code, const std::string& reason) {
        if (currentState == WebSocketState::Closed) {
            return;
        }

        running = false;
        currentState = WebSocketState::Closing;

        // Send close frame
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>((code >> 8) & 0xFF));
        payload.push_back(static_cast<uint8_t>(code & 0xFF));
        payload.insert(payload.end(), reason.begin(), reason.end());
        sendFrame(0x08, payload);

        if (socket != INVALID_SOCKET_VALUE) {
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
        }

        if (receiveThread.joinable()) {
            receiveThread.join();
        }

        currentState = WebSocketState::Closed;
    }
};


WebSocket::WebSocket() : m_impl(std::make_shared<WebSocketImpl>()) {}

WebSocket WebSocket::connect(const std::string& url) {
    WebSocket ws;
    ws.m_impl->url = url;
    return ws;
}

WebSocket& WebSocket::onOpen(std::function<void()> callback) {
    m_impl->onOpenCallback = std::move(callback);
    return *this;
}

WebSocket& WebSocket::onMessage(std::function<void(const std::string&)> callback) {
    m_impl->onMessageCallback = std::move(callback);
    return *this;
}

WebSocket& WebSocket::onBinaryMessage(std::function<void(const std::vector<uint8_t>&)> callback) {
    m_impl->onBinaryMessageCallback = std::move(callback);
    return *this;
}

WebSocket& WebSocket::onClose(std::function<void(int, const std::string&)> callback) {
    m_impl->onCloseCallback = std::move(callback);
    return *this;
}

WebSocket& WebSocket::onError(std::function<void(const std::string&)> callback) {
    m_impl->onErrorCallback = std::move(callback);
    return *this;
}

void WebSocket::send(const std::string& message) {
    if (m_impl) m_impl->send(message);
}

void WebSocket::send(const std::vector<uint8_t>& data) {
    if (m_impl) m_impl->send(data);
}

void WebSocket::ping() {
    if (m_impl) m_impl->sendFrame(0x09, {});
}

void WebSocket::close(int code, const std::string& reason) {
    if (m_impl) m_impl->close(code, reason);
}

WebSocket& WebSocket::autoReconnect(bool enabled) {
    m_impl->autoReconnectEnabled = enabled;
    return *this;
}

WebSocket& WebSocket::reconnectDelay(int milliseconds) {
    m_impl->reconnectDelayMs = milliseconds;
    return *this;
}

WebSocket& WebSocket::maxReconnectAttempts(int attempts) {
    m_impl->maxReconnectAttempts = attempts;
    return *this;
}

WebSocket& WebSocket::subprotocol(const std::string& protocol) {
    m_impl->subprotocol = protocol;
    return *this;
}

WebSocket& WebSocket::header(const std::string& key, const std::string& value) {
    m_impl->headers[key] = value;
    return *this;
}

WebSocketState WebSocket::state() const {
    return m_impl ? m_impl->currentState : WebSocketState::Closed;
}

bool WebSocket::isConnected() const {
    return m_impl && m_impl->currentState == WebSocketState::Open;
}

WebSocketHandle WebSocket::build() {
    if (m_impl->connect()) {
        return m_impl;
    }
    return nullptr;
}


// =============================================================================
// TCP Socket Implementation
// =============================================================================

class TcpSocketImpl : public std::enable_shared_from_this<TcpSocketImpl> {
public:
    SocketType socket = INVALID_SOCKET_VALUE;
    SocketState currentState = SocketState::Disconnected;
    SocketAddress localAddr;
    SocketAddress remoteAddr;
    
    std::function<void(const std::vector<uint8_t>&)> onDataCallback;
    std::function<void()> onDisconnectCallback;
    std::function<void(const std::string&)> onErrorCallback;
    
    std::atomic<bool> running{false};
    std::thread receiveThread;
    std::mutex sendMutex;

    // Socket options
    bool noDelayEnabled = false;
    bool keepAliveEnabled = false;
    int recvTimeoutMs = 0;
    int sendTimeoutMs = 0;
    int recvBufferSize = 0;
    int sendBufferSize = 0;

    ~TcpSocketImpl() {
        disconnect();
    }

    bool connect(const std::string& host, uint16_t port) {
        initialize();
        currentState = SocketState::Connecting;

        socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket == INVALID_SOCKET_VALUE) {
            currentState = SocketState::Error;
            if (onErrorCallback) onErrorCallback("Failed to create socket");
            return false;
        }

        applySocketOptions();

        struct addrinfo hints = {}, *result = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0) {
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
            currentState = SocketState::Error;
            if (onErrorCallback) onErrorCallback("Failed to resolve host");
            return false;
        }

        if (::connect(socket, result->ai_addr, static_cast<int>(result->ai_addrlen)) != 0) {
            freeaddrinfo(result);
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
            currentState = SocketState::Error;
            if (onErrorCallback) onErrorCallback("Failed to connect");
            return false;
        }

        freeaddrinfo(result);
        remoteAddr = {host, port};
        currentState = SocketState::Connected;
        running = true;

        // Get local address
        struct sockaddr_in localSockAddr;
        socklen_t addrLen = sizeof(localSockAddr);
        if (getsockname(socket, reinterpret_cast<struct sockaddr*>(&localSockAddr), &addrLen) == 0) {
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &localSockAddr.sin_addr, ipStr, sizeof(ipStr));
            localAddr = {ipStr, ntohs(localSockAddr.sin_port)};
        }

        // Start receive thread if callback is set
        if (onDataCallback) {
            receiveThread = std::thread([this]() { receiveLoop(); });
        }

        return true;
    }

    void applySocketOptions() {
        if (socket == INVALID_SOCKET_VALUE) return;

        if (noDelayEnabled) {
            int flag = 1;
            setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, 
                      reinterpret_cast<const char*>(&flag), sizeof(flag));
        }

        if (keepAliveEnabled) {
            int flag = 1;
            setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE,
                      reinterpret_cast<const char*>(&flag), sizeof(flag));
        }

        if (recvTimeoutMs > 0) {
#ifdef _WIN32
            DWORD timeout = recvTimeoutMs;
            setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,
                      reinterpret_cast<const char*>(&timeout), sizeof(timeout));
#else
            struct timeval tv;
            tv.tv_sec = recvTimeoutMs / 1000;
            tv.tv_usec = (recvTimeoutMs % 1000) * 1000;
            setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,
                      reinterpret_cast<const char*>(&tv), sizeof(tv));
#endif
        }

        if (sendTimeoutMs > 0) {
#ifdef _WIN32
            DWORD timeout = sendTimeoutMs;
            setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO,
                      reinterpret_cast<const char*>(&timeout), sizeof(timeout));
#else
            struct timeval tv;
            tv.tv_sec = sendTimeoutMs / 1000;
            tv.tv_usec = (sendTimeoutMs % 1000) * 1000;
            setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO,
                      reinterpret_cast<const char*>(&tv), sizeof(tv));
#endif
        }

        if (recvBufferSize > 0) {
            setsockopt(socket, SOL_SOCKET, SO_RCVBUF,
                      reinterpret_cast<const char*>(&recvBufferSize), sizeof(recvBufferSize));
        }

        if (sendBufferSize > 0) {
            setsockopt(socket, SOL_SOCKET, SO_SNDBUF,
                      reinterpret_cast<const char*>(&sendBufferSize), sizeof(sendBufferSize));
        }
    }

    bool listen(uint16_t port, int backlog) {
        initialize();

        socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket == INVALID_SOCKET_VALUE) {
            currentState = SocketState::Error;
            if (onErrorCallback) onErrorCallback("Failed to create socket");
            return false;
        }

        int reuseAddr = 1;
        setsockopt(socket, SOL_SOCKET, SO_REUSEADDR,
                  reinterpret_cast<const char*>(&reuseAddr), sizeof(reuseAddr));

        applySocketOptions();

        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (::bind(socket, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0) {
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
            currentState = SocketState::Error;
            if (onErrorCallback) onErrorCallback("Failed to bind socket");
            return false;
        }

        if (::listen(socket, backlog) != 0) {
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
            currentState = SocketState::Error;
            if (onErrorCallback) onErrorCallback("Failed to listen");
            return false;
        }

        localAddr = {"0.0.0.0", port};
        currentState = SocketState::Listening;
        running = true;

        return true;
    }

    std::shared_ptr<TcpSocketImpl> accept() {
        if (currentState != SocketState::Listening) {
            return nullptr;
        }

        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        SocketType clientSocket = ::accept(socket, 
            reinterpret_cast<struct sockaddr*>(&clientAddr), &addrLen);

        if (clientSocket == INVALID_SOCKET_VALUE) {
            return nullptr;
        }

        auto client = std::make_shared<TcpSocketImpl>();
        client->socket = clientSocket;
        client->currentState = SocketState::Connected;
        client->running = true;

        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));
        client->remoteAddr = {ipStr, ntohs(clientAddr.sin_port)};

        return client;
    }

    void receiveLoop() {
        std::vector<uint8_t> buffer(4096);

        while (running && socket != INVALID_SOCKET_VALUE) {
            int received = ::recv(socket, reinterpret_cast<char*>(buffer.data()),
                                 static_cast<int>(buffer.size()), 0);

            if (received <= 0) {
                if (running) {
                    running = false;
                    currentState = SocketState::Disconnected;
                    if (onDisconnectCallback) onDisconnectCallback();
                }
                break;
            }

            if (onDataCallback) {
                onDataCallback(std::vector<uint8_t>(buffer.begin(), buffer.begin() + received));
            }
        }
    }

    void disconnect() {
        running = false;
        currentState = SocketState::Disconnected;

        if (socket != INVALID_SOCKET_VALUE) {
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
        }

        if (receiveThread.joinable()) {
            receiveThread.join();
        }
    }

    int send(const void* data, size_t size) {
        if (socket == INVALID_SOCKET_VALUE || currentState != SocketState::Connected) {
            return -1;
        }

        std::lock_guard<std::mutex> lock(sendMutex);
        return ::send(socket, static_cast<const char*>(data), static_cast<int>(size), 0);
    }

    int receive(void* buffer, size_t size) {
        if (socket == INVALID_SOCKET_VALUE || currentState != SocketState::Connected) {
            return -1;
        }

        return ::recv(socket, static_cast<char*>(buffer), static_cast<int>(size), 0);
    }
};


TcpSocket::TcpSocket() : m_impl(std::make_shared<TcpSocketImpl>()) {}

TcpSocket TcpSocket::create() {
    return TcpSocket();
}

bool TcpSocket::connect(const std::string& host, uint16_t port) {
    return m_impl->connect(host, port);
}

bool TcpSocket::connect(const SocketAddress& address) {
    return m_impl->connect(address.host, address.port);
}

void TcpSocket::connectAsync(const std::string& host, uint16_t port,
                             std::function<void(bool)> callback) {
    std::thread([this, host, port, callback]() {
        bool result = connect(host, port);
        if (callback) callback(result);
    }).detach();
}

void TcpSocket::disconnect() {
    m_impl->disconnect();
}

bool TcpSocket::listen(uint16_t port, int backlog) {
    return m_impl->listen(port, backlog);
}

TcpSocketHandle TcpSocket::accept() {
    return m_impl->accept();
}

void TcpSocket::acceptAsync(std::function<void(TcpSocketHandle)> callback) {
    std::thread([this, callback]() {
        auto client = accept();
        if (callback) callback(client);
    }).detach();
}

int TcpSocket::send(const void* data, size_t size) {
    return m_impl->send(data, size);
}

int TcpSocket::send(const std::string& data) {
    return m_impl->send(data.data(), data.size());
}

int TcpSocket::send(const std::vector<uint8_t>& data) {
    return m_impl->send(data.data(), data.size());
}

int TcpSocket::receive(void* buffer, size_t size) {
    return m_impl->receive(buffer, size);
}

std::string TcpSocket::receiveString(size_t maxSize) {
    std::vector<char> buffer(maxSize);
    int received = m_impl->receive(buffer.data(), maxSize);
    if (received > 0) {
        return std::string(buffer.data(), received);
    }
    return "";
}

std::vector<uint8_t> TcpSocket::receiveBytes(size_t maxSize) {
    std::vector<uint8_t> buffer(maxSize);
    int received = m_impl->receive(buffer.data(), maxSize);
    if (received > 0) {
        buffer.resize(received);
        return buffer;
    }
    return {};
}

void TcpSocket::sendAsync(const std::vector<uint8_t>& data,
                          std::function<void(int)> callback) {
    std::thread([this, data, callback]() {
        int sent = send(data);
        if (callback) callback(sent);
    }).detach();
}

void TcpSocket::receiveAsync(size_t maxSize,
                             std::function<void(std::vector<uint8_t>)> callback) {
    std::thread([this, maxSize, callback]() {
        auto data = receiveBytes(maxSize);
        if (callback) callback(data);
    }).detach();
}

TcpSocket& TcpSocket::onData(std::function<void(const std::vector<uint8_t>&)> callback) {
    m_impl->onDataCallback = std::move(callback);
    return *this;
}

TcpSocket& TcpSocket::onDisconnect(std::function<void()> callback) {
    m_impl->onDisconnectCallback = std::move(callback);
    return *this;
}

TcpSocket& TcpSocket::onError(std::function<void(const std::string&)> callback) {
    m_impl->onErrorCallback = std::move(callback);
    return *this;
}

TcpSocket& TcpSocket::noDelay(bool enabled) {
    m_impl->noDelayEnabled = enabled;
    return *this;
}

TcpSocket& TcpSocket::keepAlive(bool enabled) {
    m_impl->keepAliveEnabled = enabled;
    return *this;
}

TcpSocket& TcpSocket::receiveTimeout(int milliseconds) {
    m_impl->recvTimeoutMs = milliseconds;
    return *this;
}

TcpSocket& TcpSocket::sendTimeout(int milliseconds) {
    m_impl->sendTimeoutMs = milliseconds;
    return *this;
}

TcpSocket& TcpSocket::receiveBufferSize(int size) {
    m_impl->recvBufferSize = size;
    return *this;
}

TcpSocket& TcpSocket::sendBufferSize(int size) {
    m_impl->sendBufferSize = size;
    return *this;
}

SocketState TcpSocket::state() const {
    return m_impl->currentState;
}

bool TcpSocket::isConnected() const {
    return m_impl->currentState == SocketState::Connected;
}

SocketAddress TcpSocket::localAddress() const {
    return m_impl->localAddr;
}

SocketAddress TcpSocket::remoteAddress() const {
    return m_impl->remoteAddr;
}

TcpSocketHandle TcpSocket::build() {
    return m_impl;
}


// =============================================================================
// UDP Socket Implementation
// =============================================================================

class UdpSocketImpl {
public:
    SocketType socket = INVALID_SOCKET_VALUE;
    SocketAddress localAddr;
    bool bound = false;
    
    std::function<void(const std::vector<uint8_t>&, const SocketAddress&)> onDataCallback;
    std::function<void(const std::string&)> onErrorCallback;
    
    std::atomic<bool> running{false};
    std::thread receiveThread;
    std::mutex sendMutex;

    // Socket options
    bool broadcastEnabled = false;
    int recvBufferSize = 0;
    int sendBufferSize = 0;

    ~UdpSocketImpl() {
        running = false;
        if (socket != INVALID_SOCKET_VALUE) {
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
        }
        if (receiveThread.joinable()) {
            receiveThread.join();
        }
    }

    bool bind(const std::string& host, uint16_t port) {
        initialize();

        socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket == INVALID_SOCKET_VALUE) {
            if (onErrorCallback) onErrorCallback("Failed to create socket");
            return false;
        }

        applySocketOptions();

        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (host.empty() || host == "0.0.0.0") {
            addr.sin_addr.s_addr = INADDR_ANY;
        } else {
            inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
        }

        if (::bind(socket, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0) {
            CloseSocketFunc(socket);
            socket = INVALID_SOCKET_VALUE;
            if (onErrorCallback) onErrorCallback("Failed to bind socket");
            return false;
        }

        localAddr = {host.empty() ? "0.0.0.0" : host, port};
        bound = true;
        running = true;

        // Start receive thread if callback is set
        if (onDataCallback) {
            receiveThread = std::thread([this]() { receiveLoop(); });
        }

        return true;
    }

    void applySocketOptions() {
        if (socket == INVALID_SOCKET_VALUE) return;

        if (broadcastEnabled) {
            int flag = 1;
            setsockopt(socket, SOL_SOCKET, SO_BROADCAST,
                      reinterpret_cast<const char*>(&flag), sizeof(flag));
        }

        if (recvBufferSize > 0) {
            setsockopt(socket, SOL_SOCKET, SO_RCVBUF,
                      reinterpret_cast<const char*>(&recvBufferSize), sizeof(recvBufferSize));
        }

        if (sendBufferSize > 0) {
            setsockopt(socket, SOL_SOCKET, SO_SNDBUF,
                      reinterpret_cast<const char*>(&sendBufferSize), sizeof(sendBufferSize));
        }
    }

    void receiveLoop() {
        std::vector<uint8_t> buffer(65535);

        while (running && socket != INVALID_SOCKET_VALUE) {
            struct sockaddr_in senderAddr;
            socklen_t addrLen = sizeof(senderAddr);

            int received = recvfrom(socket, reinterpret_cast<char*>(buffer.data()),
                                   static_cast<int>(buffer.size()), 0,
                                   reinterpret_cast<struct sockaddr*>(&senderAddr), &addrLen);

            if (received > 0 && onDataCallback) {
                char ipStr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));
                SocketAddress sender{ipStr, ntohs(senderAddr.sin_port)};
                
                onDataCallback(std::vector<uint8_t>(buffer.begin(), buffer.begin() + received), sender);
            }
        }
    }

    int sendTo(const void* data, size_t size, const SocketAddress& address) {
        if (socket == INVALID_SOCKET_VALUE) {
            // Create socket if not bound
            socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (socket == INVALID_SOCKET_VALUE) {
                return -1;
            }
            applySocketOptions();
        }

        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(address.port);
        inet_pton(AF_INET, address.host.c_str(), &addr.sin_addr);

        std::lock_guard<std::mutex> lock(sendMutex);
        return sendto(socket, static_cast<const char*>(data), static_cast<int>(size), 0,
                     reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    }

    int receiveFrom(void* buffer, size_t size, SocketAddress& sender) {
        if (socket == INVALID_SOCKET_VALUE) {
            return -1;
        }

        struct sockaddr_in senderAddr;
        socklen_t addrLen = sizeof(senderAddr);

        int received = recvfrom(socket, static_cast<char*>(buffer), static_cast<int>(size), 0,
                               reinterpret_cast<struct sockaddr*>(&senderAddr), &addrLen);

        if (received > 0) {
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));
            sender = {ipStr, ntohs(senderAddr.sin_port)};
        }

        return received;
    }

    bool multicastJoin(const std::string& group) {
        if (socket == INVALID_SOCKET_VALUE) return false;

        struct ip_mreq mreq;
        inet_pton(AF_INET, group.c_str(), &mreq.imr_multiaddr);
        mreq.imr_interface.s_addr = INADDR_ANY;

        return setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                         reinterpret_cast<const char*>(&mreq), sizeof(mreq)) == 0;
    }

    bool multicastLeave(const std::string& group) {
        if (socket == INVALID_SOCKET_VALUE) return false;

        struct ip_mreq mreq;
        inet_pton(AF_INET, group.c_str(), &mreq.imr_multiaddr);
        mreq.imr_interface.s_addr = INADDR_ANY;

        return setsockopt(socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                         reinterpret_cast<const char*>(&mreq), sizeof(mreq)) == 0;
    }

    bool multicastTTL(int ttl) {
        if (socket == INVALID_SOCKET_VALUE) return false;

        return setsockopt(socket, IPPROTO_IP, IP_MULTICAST_TTL,
                         reinterpret_cast<const char*>(&ttl), sizeof(ttl)) == 0;
    }
};


UdpSocket::UdpSocket() : m_impl(std::make_shared<UdpSocketImpl>()) {}

UdpSocket UdpSocket::create() {
    return UdpSocket();
}

bool UdpSocket::bind(uint16_t port) {
    return m_impl->bind("", port);
}

bool UdpSocket::bind(const std::string& host, uint16_t port) {
    return m_impl->bind(host, port);
}

int UdpSocket::sendTo(const void* data, size_t size, const SocketAddress& address) {
    return m_impl->sendTo(data, size, address);
}

int UdpSocket::sendTo(const std::string& data, const SocketAddress& address) {
    return m_impl->sendTo(data.data(), data.size(), address);
}

int UdpSocket::sendTo(const std::vector<uint8_t>& data, const SocketAddress& address) {
    return m_impl->sendTo(data.data(), data.size(), address);
}

int UdpSocket::receiveFrom(void* buffer, size_t size, SocketAddress& sender) {
    return m_impl->receiveFrom(buffer, size, sender);
}

std::pair<std::vector<uint8_t>, SocketAddress> UdpSocket::receiveFrom(size_t maxSize) {
    std::vector<uint8_t> buffer(maxSize);
    SocketAddress sender;
    int received = m_impl->receiveFrom(buffer.data(), maxSize, sender);
    if (received > 0) {
        buffer.resize(received);
        return {buffer, sender};
    }
    return {{}, {}};
}

void UdpSocket::sendToAsync(const std::vector<uint8_t>& data, const SocketAddress& address,
                            std::function<void(int)> callback) {
    std::thread([this, data, address, callback]() {
        int sent = sendTo(data, address);
        if (callback) callback(sent);
    }).detach();
}

void UdpSocket::receiveFromAsync(size_t maxSize,
                                 std::function<void(std::vector<uint8_t>, SocketAddress)> callback) {
    std::thread([this, maxSize, callback]() {
        auto [data, sender] = receiveFrom(maxSize);
        if (callback) callback(data, sender);
    }).detach();
}

UdpSocket& UdpSocket::onData(std::function<void(const std::vector<uint8_t>&, const SocketAddress&)> callback) {
    m_impl->onDataCallback = std::move(callback);
    return *this;
}

UdpSocket& UdpSocket::onError(std::function<void(const std::string&)> callback) {
    m_impl->onErrorCallback = std::move(callback);
    return *this;
}

UdpSocket& UdpSocket::broadcast(bool enabled) {
    m_impl->broadcastEnabled = enabled;
    return *this;
}

UdpSocket& UdpSocket::multicastJoin(const std::string& group) {
    m_impl->multicastJoin(group);
    return *this;
}

UdpSocket& UdpSocket::multicastLeave(const std::string& group) {
    m_impl->multicastLeave(group);
    return *this;
}

UdpSocket& UdpSocket::multicastTTL(int ttl) {
    m_impl->multicastTTL(ttl);
    return *this;
}

UdpSocket& UdpSocket::receiveBufferSize(int size) {
    m_impl->recvBufferSize = size;
    return *this;
}

UdpSocket& UdpSocket::sendBufferSize(int size) {
    m_impl->sendBufferSize = size;
    return *this;
}

bool UdpSocket::isBound() const {
    return m_impl->bound;
}

SocketAddress UdpSocket::localAddress() const {
    return m_impl->localAddr;
}

UdpSocketHandle UdpSocket::build() {
    return m_impl;
}


// =============================================================================
// FileDownloader Implementation
// =============================================================================

class FileDownloaderImpl {
public:
    std::string url;
    std::string destination;
    std::map<std::string, std::string> headers;
    int timeoutMs = 30000;
    int maxRetries = 3;
    size_t chunkSize = 65536;
    
    DownloadState currentState = DownloadState::Idle;
    DownloadProgress currentProgress;
    std::string errorMessage;
    
    std::function<void(DownloadProgress)> onProgressCallback;
    std::function<void(bool, const std::string&)> onCompleteCallback;
    std::function<void(DownloadState)> onStateChangeCallback;
    
    std::atomic<bool> running{false};
    std::atomic<bool> paused{false};
    std::thread downloadThread;
    std::mutex stateMutex;
    
    size_t resumePosition = 0;

    ~FileDownloaderImpl() {
        cancel();
    }

    void setState(DownloadState state) {
        std::lock_guard<std::mutex> lock(stateMutex);
        currentState = state;
        if (onStateChangeCallback) {
            onStateChangeCallback(state);
        }
    }

    bool start(const std::string& downloadUrl, const std::string& dest) {
        if (currentState == DownloadState::Downloading) {
            return false;
        }

        url = downloadUrl;
        destination = dest;
        resumePosition = 0;
        currentProgress = {};
        errorMessage.clear();

        running = true;
        paused = false;
        setState(DownloadState::Downloading);

        downloadThread = std::thread([this]() { downloadLoop(); });
        return true;
    }

    void downloadLoop() {
        initialize();
        int retries = 0;

        while (running && retries <= maxRetries) {
            // Check for pause
            while (paused && running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            if (!running) break;

#ifdef _WIN32
            if (downloadWithWinHttp()) {
                setState(DownloadState::Completed);
                if (onCompleteCallback) {
                    onCompleteCallback(true, "");
                }
                return;
            }
#else
#ifdef KGK_HAS_CURL
            if (downloadWithCurl()) {
                setState(DownloadState::Completed);
                if (onCompleteCallback) {
                    onCompleteCallback(true, "");
                }
                return;
            }
#else
            errorMessage = "Download not supported on this platform";
            break;
#endif
#endif

            if (!running) break;
            ++retries;
            if (retries <= maxRetries) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        if (running) {
            setState(DownloadState::Failed);
            if (onCompleteCallback) {
                onCompleteCallback(false, errorMessage);
            }
        }
    }

#ifdef _WIN32
    bool downloadWithWinHttp() {
        HINTERNET hSession = WinHttpOpen(L"KGKNet/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

        if (!hSession) {
            errorMessage = "Failed to initialize HTTP session";
            return false;
        }

        // Parse URL
        URL_COMPONENTS urlComp = {};
        urlComp.dwStructSize = sizeof(urlComp);
        urlComp.dwSchemeLength = -1;
        urlComp.dwHostNameLength = -1;
        urlComp.dwUrlPathLength = -1;
        urlComp.dwExtraInfoLength = -1;

        std::wstring wUrl(url.begin(), url.end());
        if (!WinHttpCrackUrl(wUrl.c_str(), 0, 0, &urlComp)) {
            WinHttpCloseHandle(hSession);
            errorMessage = "Invalid URL";
            return false;
        }

        std::wstring hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);
        std::wstring urlPath(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
        if (urlComp.dwExtraInfoLength > 0) {
            urlPath += std::wstring(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);
        }

        bool isHttps = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);
        INTERNET_PORT port = urlComp.nPort ? urlComp.nPort : (isHttps ? 443 : 80);

        HINTERNET hConnect = WinHttpConnect(hSession, hostName.c_str(), port, 0);
        if (!hConnect) {
            WinHttpCloseHandle(hSession);
            errorMessage = "Failed to connect";
            return false;
        }

        DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlPath.c_str(),
            nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);

        if (!hRequest) {
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            errorMessage = "Failed to create request";
            return false;
        }

        // Add Range header for resume
        if (resumePosition > 0) {
            std::wstring rangeHeader = L"Range: bytes=" + std::to_wstring(resumePosition) + L"-";
            WinHttpAddRequestHeaders(hRequest, rangeHeader.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);
        }

        // Add custom headers
        for (const auto& [key, value] : headers) {
            std::wstring header = std::wstring(key.begin(), key.end()) + L": " +
                                 std::wstring(value.begin(), value.end());
            WinHttpAddRequestHeaders(hRequest, header.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);
        }

        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                               WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            errorMessage = "Failed to send request";
            return false;
        }

        if (!WinHttpReceiveResponse(hRequest, nullptr)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            errorMessage = "Failed to receive response";
            return false;
        }

        // Get content length
        DWORD contentLength = 0;
        DWORD size = sizeof(contentLength);
        WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &contentLength, &size, WINHTTP_NO_HEADER_INDEX);

        currentProgress.totalBytes = contentLength + resumePosition;

        // Open file
        std::ofstream file(destination, 
            resumePosition > 0 ? (std::ios::binary | std::ios::app) : std::ios::binary);
        if (!file) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            errorMessage = "Failed to open file";
            return false;
        }

        currentProgress.bytesDownloaded = resumePosition;
        auto startTime = std::chrono::steady_clock::now();

        std::vector<char> buffer(chunkSize);
        DWORD bytesAvailable = 0;
        DWORD bytesRead = 0;

        while (running && !paused) {
            if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable) || bytesAvailable == 0) {
                break;
            }

            DWORD toRead = std::min(bytesAvailable, static_cast<DWORD>(buffer.size()));
            if (!WinHttpReadData(hRequest, buffer.data(), toRead, &bytesRead) || bytesRead == 0) {
                break;
            }

            file.write(buffer.data(), bytesRead);
            currentProgress.bytesDownloaded += bytesRead;

            // Calculate progress
            if (currentProgress.totalBytes > 0) {
                currentProgress.percentage = 
                    static_cast<float>(currentProgress.bytesDownloaded) / 
                    static_cast<float>(currentProgress.totalBytes) * 100.0f;
            }

            // Calculate speed
            auto elapsed = std::chrono::steady_clock::now() - startTime;
            auto elapsedSec = std::chrono::duration<float>(elapsed).count();
            if (elapsedSec > 0) {
                currentProgress.speedBytesPerSec = 
                    static_cast<float>(currentProgress.bytesDownloaded - resumePosition) / elapsedSec;
                
                if (currentProgress.speedBytesPerSec > 0 && currentProgress.totalBytes > 0) {
                    size_t remaining = currentProgress.totalBytes - currentProgress.bytesDownloaded;
                    currentProgress.estimatedSecondsRemaining = 
                        static_cast<int>(remaining / currentProgress.speedBytesPerSec);
                }
            }

            if (onProgressCallback) {
                onProgressCallback(currentProgress);
            }
        }

        file.close();
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        if (paused) {
            resumePosition = currentProgress.bytesDownloaded;
            return false;
        }

        return running && currentProgress.bytesDownloaded > 0;
    }
#endif

#if !defined(_WIN32) && defined(KGK_HAS_CURL)
    bool downloadWithCurl() {
        CURL* curl = curl_easy_init();
        if (!curl) {
            errorMessage = "Failed to initialize CURL";
            return false;
        }

        std::ofstream file(destination,
            resumePosition > 0 ? (std::ios::binary | std::ios::app) : std::ios::binary);
        if (!file) {
            curl_easy_cleanup(curl);
            errorMessage = "Failed to open file";
            return false;
        }

        auto startTime = std::chrono::steady_clock::now();
        currentProgress.bytesDownloaded = resumePosition;

        struct WriteData {
            std::ofstream* file;
            FileDownloaderImpl* impl;
            std::chrono::steady_clock::time_point startTime;
        } writeData{&file, this, startTime};

        auto writeCallback = [](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
            auto* data = static_cast<WriteData*>(userdata);
            size_t bytes = size * nmemb;

            if (!data->impl->running || data->impl->paused) {
                return 0; // Abort
            }

            data->file->write(ptr, bytes);
            data->impl->currentProgress.bytesDownloaded += bytes;

            // Calculate progress
            if (data->impl->currentProgress.totalBytes > 0) {
                data->impl->currentProgress.percentage =
                    static_cast<float>(data->impl->currentProgress.bytesDownloaded) /
                    static_cast<float>(data->impl->currentProgress.totalBytes) * 100.0f;
            }

            // Calculate speed
            auto elapsed = std::chrono::steady_clock::now() - data->startTime;
            auto elapsedSec = std::chrono::duration<float>(elapsed).count();
            if (elapsedSec > 0) {
                data->impl->currentProgress.speedBytesPerSec =
                    static_cast<float>(data->impl->currentProgress.bytesDownloaded - 
                                      data->impl->resumePosition) / elapsedSec;
            }

            if (data->impl->onProgressCallback) {
                data->impl->onProgressCallback(data->impl->currentProgress);
            }

            return bytes;
        };

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs);

        if (resumePosition > 0) {
            curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, 
                           static_cast<curl_off_t>(resumePosition));
        }

        // Add headers
        struct curl_slist* headerList = nullptr;
        for (const auto& [key, value] : headers) {
            std::string header = key + ": " + value;
            headerList = curl_slist_append(headerList, header.c_str());
        }
        if (headerList) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
        }

        CURLcode res = curl_easy_perform(curl);

        if (headerList) {
            curl_slist_free_all(headerList);
        }

        file.close();

        if (res != CURLE_OK) {
            errorMessage = curl_easy_strerror(res);
            curl_easy_cleanup(curl);
            
            if (paused) {
                resumePosition = currentProgress.bytesDownloaded;
            }
            return false;
        }

        // Get content length if not known
        if (currentProgress.totalBytes == 0) {
            double contentLength = 0;
            curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);
            if (contentLength > 0) {
                currentProgress.totalBytes = static_cast<size_t>(contentLength);
            }
        }

        curl_easy_cleanup(curl);
        return running && currentProgress.bytesDownloaded > 0;
    }
#endif

    void pause() {
        if (currentState == DownloadState::Downloading) {
            paused = true;
            setState(DownloadState::Paused);
        }
    }

    void resume() {
        if (currentState == DownloadState::Paused) {
            paused = false;
            setState(DownloadState::Downloading);
        }
    }

    void cancel() {
        running = false;
        paused = false;
        
        if (downloadThread.joinable()) {
            downloadThread.join();
        }

        if (currentState != DownloadState::Completed) {
            setState(DownloadState::Cancelled);
        }
    }
};


FileDownloader::FileDownloader() : m_impl(std::make_unique<FileDownloaderImpl>()) {}
FileDownloader::~FileDownloader() = default;

bool FileDownloader::download(const std::string& url, const std::string& destination) {
    return m_impl->start(url, destination);
}

FileDownloader& FileDownloader::onProgress(std::function<void(DownloadProgress)> callback) {
    m_impl->onProgressCallback = std::move(callback);
    return *this;
}

FileDownloader& FileDownloader::onComplete(std::function<void(bool, const std::string&)> callback) {
    m_impl->onCompleteCallback = std::move(callback);
    return *this;
}

FileDownloader& FileDownloader::onStateChange(std::function<void(DownloadState)> callback) {
    m_impl->onStateChangeCallback = std::move(callback);
    return *this;
}

void FileDownloader::pause() {
    m_impl->pause();
}

void FileDownloader::resume() {
    m_impl->resume();
}

void FileDownloader::cancel() {
    m_impl->cancel();
}

FileDownloader& FileDownloader::timeout(int milliseconds) {
    m_impl->timeoutMs = milliseconds;
    return *this;
}

FileDownloader& FileDownloader::header(const std::string& key, const std::string& value) {
    m_impl->headers[key] = value;
    return *this;
}

FileDownloader& FileDownloader::maxRetries(int retries) {
    m_impl->maxRetries = retries;
    return *this;
}

FileDownloader& FileDownloader::chunkSize(size_t size) {
    m_impl->chunkSize = size;
    return *this;
}

DownloadState FileDownloader::state() const {
    return m_impl->currentState;
}

DownloadProgress FileDownloader::progress() const {
    return m_impl->currentProgress;
}

DownloadInfo FileDownloader::info() const {
    DownloadInfo info;
    info.url = m_impl->url;
    info.destination = m_impl->destination;
    info.state = m_impl->currentState;
    info.progress = m_impl->currentProgress;
    info.error = m_impl->errorMessage;
    return info;
}

} // namespace KGKNet
