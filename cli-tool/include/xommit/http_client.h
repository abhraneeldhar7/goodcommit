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
    HINTERNET hSession = InternetOpenA("xommit/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
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

static bool http_download_file(const std::string& url, const std::string& out_path) {
    URL_COMPONENTSA urlComp = {};
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.dwSchemeLength = 1;
    urlComp.dwHostNameLength = 1;
    urlComp.dwUrlPathLength = 1;
    urlComp.dwExtraInfoLength = 1;

    if (!InternetCrackUrlA(url.c_str(), (DWORD)url.size(), 0, &urlComp)) {
        return false;
    }

    std::string host(urlComp.lpszHostName, urlComp.dwHostNameLength);
    std::string path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
    if (urlComp.dwExtraInfoLength > 0) {
        path += std::string(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);
    }

    HINTERNET hSession = InternetOpenA("xommit/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hSession) return false;

    INTERNET_PORT port = urlComp.nScheme == INTERNET_SCHEME_HTTPS ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    HINTERNET hConnect = InternetConnectA(hSession, host.c_str(), port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) { InternetCloseHandle(hSession); return false; }

    const char* accept_types[] = { "*/*", NULL };
    DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
    if (urlComp.nScheme == INTERNET_SCHEME_HTTPS) flags |= INTERNET_FLAG_SECURE;

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "GET", path.c_str(), NULL, NULL, accept_types, flags, 0);
    if (!hRequest) { InternetCloseHandle(hConnect); InternetCloseHandle(hSession); return false; }

    if (urlComp.nScheme == INTERNET_SCHEME_HTTPS) {
        DWORD sec_flags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
        InternetSetOptionA(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &sec_flags, sizeof(sec_flags));
    }

    if (!HttpSendRequestA(hRequest, NULL, 0, NULL, 0)) {
        InternetCloseHandle(hRequest); InternetCloseHandle(hConnect); InternetCloseHandle(hSession);
        return false;
    }

    DWORD status = 0;
    DWORD status_size = sizeof(status);
    if (!HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &status_size, NULL) || status != 200) {
        InternetCloseHandle(hRequest); InternetCloseHandle(hConnect); InternetCloseHandle(hSession);
        return false;
    }

    FILE* fp = fopen(out_path.c_str(), "wb");
    if (!fp) { InternetCloseHandle(hRequest); InternetCloseHandle(hConnect); InternetCloseHandle(hSession); return false; }

    char buf[4096];
    DWORD bytes_read = 0;
    while (InternetReadFile(hRequest, buf, sizeof(buf), &bytes_read) && bytes_read > 0) {
        fwrite(buf, 1, bytes_read, fp);
        bytes_read = 0;
    }

    fclose(fp);
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);
    return true;
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

static bool http_download_file(const std::string& url, const std::string& out_path) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    FILE* fp = fopen(out_path.c_str(), "wb");
    if (!fp) { curl_easy_cleanup(curl); return false; }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    CURLcode res = curl_easy_perform(curl);
    long code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(curl);
    fclose(fp);

    return res == CURLE_OK && code == 200;
}

#endif

#endif
