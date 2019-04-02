#include "mcc/map/CurlDownloader.h"

#include <bmcl/Buffer.h>

#include <curl/curl.h>

#include <string>

namespace mccmap {

class CurlGlobalInitializer {
public:
    CurlGlobalInitializer()
    {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    ~CurlGlobalInitializer()
    {
        curl_global_cleanup();
    }
};

struct ReqData {
    explicit ReqData(CURL* handle)
        : handle(handle)
    {
    }
    bmcl::Buffer buf;
    CURL* handle;
};

static size_t writeCallback(void* src, size_t size, size_t nmemb, void* userData)
{
    ReqData* data = (ReqData*)userData;

    double imgSize;
    curl_easy_getinfo(data->handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &imgSize);
    if (imgSize >= 0) {
        data->buf.reserve(imgSize);
    }

    std::size_t totalSize = size * nmemb;
    data->buf.write(src, totalSize);

    return totalSize;
}

CurlDownloader::CurlDownloader()
{
    static CurlGlobalInitializer curlInitializer;
    _handle = (void*)curl_easy_init();
    curl_easy_setopt((CURL*)_handle, CURLOPT_WRITEFUNCTION, writeCallback);
    const char* userAgent = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10; rv:33.0) Gecko/20100101 Firefox/33.0";
    curl_easy_setopt((CURL*)_handle, CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt((CURL*)_handle, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt((CURL*)_handle, CURLOPT_TIMEOUT, 10);
}

CurlDownloader::~CurlDownloader()
{
    if (_handle) {
        curl_easy_cleanup((CURL*)_handle);
    }
}

CurlDownloader::CurlDownloader(CurlDownloader&& other)
    : _handle(other._handle)
{
    other._handle = nullptr;
}

CurlDownloader& CurlDownloader::operator=(CurlDownloader&& other)
{
    if (_handle) {
        curl_easy_cleanup((CURL*)_handle);
    }
    _handle = other._handle;
    other._handle = nullptr;
    return *this;
}

bmcl::Buffer CurlDownloader::download(const char* url)
{
    ReqData data((CURL*)_handle);
    curl_easy_setopt((CURL*)_handle, CURLOPT_URL, url);
    curl_easy_setopt((CURL*)_handle, CURLOPT_WRITEDATA, (void*)&data);
    CURLcode res = curl_easy_perform((CURL*)_handle);
    if (res != CURLE_OK) {
        return bmcl::Buffer();
    }
    return std::move(data.buf);
}

bmcl::Buffer CurlDownloader::download(const std::string& url)
{
    return download(url.c_str());
}
}
