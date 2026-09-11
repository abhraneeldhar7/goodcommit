#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>
#include <stdexcept>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

static std::string http_post(const std::string& url, const std::string& body, const std::string& api_key) {
    HINTERNET hSession = InternetOpenA("goodcommit/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hSession) throw std::runtime_error("InternetOpen failed");

    HINTERNET hConnect = InternetConnectA(hSession, "api.groq.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) { InternetCloseHandle(hSession); throw std::runtime_error("InternetConnect failed"); }

    const char* accept_types[] = { "application/json", NULL };
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/openai/v1/chat/completions", NULL, NULL, accept_types,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hRequest) { InternetCloseHandle(hConnect); InternetCloseHandle(hSession); throw std::runtime_error("HttpOpenRequest failed"); }

    DWORD sec_flags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
    InternetSetOptionA(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &sec_flags, sizeof(sec_flags));

    std::string headers = "Content-Type: application/json\r\nAuthorization: Bearer " + api_key;
    BOOL sent = HttpSendRequestA(hRequest, headers.c_str(), (DWORD)headers.size(), (LPVOID)body.c_str(), (DWORD)body.size());
    if (!sent) { InternetCloseHandle(hRequest); InternetCloseHandle(hConnect); InternetCloseHandle(hSession); throw std::runtime_error("HttpSendRequest failed"); }

    std::string response;
    char buf[4096];
    DWORD bytes_read = 0;
    while (InternetReadFile(hRequest, buf, sizeof(buf) - 1, &bytes_read) && bytes_read > 0) {
        buf[bytes_read] = '\0';
        response.append(buf, bytes_read);
        bytes_read = 0;
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);

    return response;
}

#else

#include <curl/curl.h>

static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t new_len = size * nmemb;
    s->append(static_cast<char*>(contents), new_len);
    return new_len;
}

static std::string http_post(const std::string& url, const std::string& body, const std::string& api_key) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl_easy_init failed");

    std::string response;
    struct curl_slist* headers = NULL;
    std::string auth = "Authorization: Bearer " + api_key;

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)body.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::string err = curl_easy_strerror(res);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw std::runtime_error("curl failed: " + err);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

#endif

#endif
