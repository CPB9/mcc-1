// based on public domain code https://github.com/tarasvb/qtcurl.git

#pragma once

#include "mcc/Config.h"

#include <bmcl/Fwd.h>

#include <QObject>

#include <curl/curl.h>

#include <map>
#include <functional>

namespace mccmap {

class CurlMulti;

class MCC_MAP_DECLSPEC CurlEasy : public QObject {
    Q_OBJECT
public:
    using WriteFunction = std::function<std::size_t(const void* buffer, std::size_t size)>;
    using HeaderFunction = WriteFunction;
    using ReadFunction = std::function<std::size_t(void* buffer, std::size_t size)>;
    using SeekFunction = std::function<int(curl_off_t offset, int origin)>;

    virtual ~CurlEasy();

    bool perform();
    bool abort();
    bool isRunning() const;

    CURLcode result() const;

    template<typename T>
    bool set(CURLoption option, T parameter);

    bool set(CURLoption option, const QString& parameter); // Convenience override for const char* parameters
    bool set(CURLoption option, const QUrl& parameter); // Convenience override for const char* parameters

    void setReadFunction(const ReadFunction& function);
    void setWriteFunction(const WriteFunction& function);
    void setHeaderFunction(const HeaderFunction& function);
    void setSeekFunction(const SeekFunction& function);

    template<typename T>
    bool get(CURLINFO info, T* pointer);
    template<typename T>
    T get(CURLINFO info);

    bmcl::Option<QString> findHttpHeader(const QString& header) const;
    void setHttpHeader(const QString& header, const QString& value);
    bool hasHttpHeader(const QString& header) const;
    bool removeHttpHeader(const QString& header);

    bmcl::Option<const QByteArray&> findHttpHeaderRaw(const QString& header) const;
    void setHttpHeaderRaw(const QString& header, const QByteArray& encodedValue);

    CURL* handle();

signals:
    void aborted();
    void progress(curl_off_t downloadTotal, curl_off_t downloadNow, curl_off_t uploadTotal, curl_off_t uploadNow);
    void done(CURLcode result);

private:
    friend class CurlMulti;

    explicit CurlEasy(CurlMulti* parent);

    void onCurlMessage(CURLMsg* message);
    void rebuildCurlHttpHeaders();

    static size_t staticCurlReadFunction(void* data, size_t size, size_t nitems, void* easyPtr);
    static size_t staticCurlWriteFunction(const void* data, size_t size, size_t nitems, void* easyPtr);
    static size_t staticCurlHeaderFunction(const void* data, size_t size, size_t nitems, void* easyPtr);
    static int staticCurlSeekFunction(void* easyPtr, curl_off_t offset, int origin);
    static int staticCurlXferInfoFunction(void* easyPtr, curl_off_t downloadTotal, curl_off_t downloadNow, curl_off_t uploadTotal, curl_off_t uploadNow);

    CURL* _handle;
    CurlMulti* _multi;
    CURLcode _lastResult;

    ReadFunction _readFunction;
    WriteFunction _writeFunction;
    HeaderFunction _headerFunction;
    SeekFunction _seekFunction;

    std::map<QString, QByteArray> _httpHeaders;

    struct curl_slist* _curlHttpHeaders;
    bool _httpHeadersWereSet;
    bool _isRunning;
};

template<typename T>
bool CurlEasy::set(CURLoption option, T parameter)
{
    return curl_easy_setopt(_handle, option, parameter) == CURLE_OK;
}

template<typename T>
bool CurlEasy::get(CURLINFO info, T* pointer)
{
    return curl_easy_getinfo(_handle, info, pointer) == CURLE_OK;
}

template<typename T>
T CurlEasy::get(CURLINFO info)
{
    T parameter;
    get(info, &parameter);
    return parameter;
}

}
