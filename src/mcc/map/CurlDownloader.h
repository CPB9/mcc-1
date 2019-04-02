#pragma once

#include "mcc/Config.h"

#include <string>

namespace bmcl {
class Buffer;
}

namespace mccmap {

class MCC_MAP_DECLSPEC CurlDownloader {
public:
    CurlDownloader();
    ~CurlDownloader();

    CurlDownloader(const CurlDownloader& other) = delete;
    CurlDownloader(CurlDownloader&& other);

    bmcl::Buffer download(const char* url);
    bmcl::Buffer download(const std::string& url);

    CurlDownloader& operator=(const CurlDownloader& other) = delete;
    CurlDownloader& operator=(CurlDownloader&& other);

private:
    void* _handle;
};
}
