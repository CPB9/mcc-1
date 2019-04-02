// based on public domain code https://github.com/tarasvb/qtcurl.git

#include "mcc/map/CurlEasy.h"
#include "mcc/map/CurlMulti.h"

#include <bmcl/Option.h>

#include <QUrl>

#include <cassert>

namespace mccmap {

CurlEasy::CurlEasy(CurlMulti* parent)
    : QObject(parent)
    , _handle(curl_easy_init())
    , _multi(parent)
    , _lastResult(CURLE_OK)
    , _curlHttpHeaders(nullptr)
    , _httpHeadersWereSet(false)
    , _isRunning(false)
{
    assert(_handle != nullptr);

    set(CURLOPT_PRIVATE, this);
    set(CURLOPT_XFERINFOFUNCTION, staticCurlXferInfoFunction);
    set(CURLOPT_XFERINFODATA, this);
    set(CURLOPT_NOPROGRESS, long(0));
}

CurlEasy::~CurlEasy()
{
    _multi->onEasyDelete(this);

    curl_easy_cleanup(_handle);

    if (_curlHttpHeaders) {
        curl_slist_free_all(_curlHttpHeaders);
        _curlHttpHeaders = nullptr;
    }
}

bool CurlEasy::perform()
{
    if (isRunning()) {
        return false;
    }

    rebuildCurlHttpHeaders();

    _multi->startEasy(this);
    _isRunning = true;
    return true;
}

bool CurlEasy::abort()
{
    if (!isRunning()) {
        return false;
    }

    _multi->stopEasy(this);
    _isRunning = false;
    emit aborted();
    return true;
}

void CurlEasy::onCurlMessage(CURLMsg* message)
{
    if (message->msg == CURLMSG_DONE) {
        _multi->stopEasy(this);
        _isRunning = false;
        _lastResult = message->data.result;
        emit done(_lastResult);
    }
}

void CurlEasy::rebuildCurlHttpHeaders()
{
    if (!_httpHeadersWereSet) {
        return;
    }

    if (_curlHttpHeaders) {
        curl_slist_free_all(_curlHttpHeaders);
        _curlHttpHeaders = nullptr;
    }

    for (auto it : _httpHeaders) {
        const QString& header = it.first;
        const QByteArray& value = it.second;

        QByteArray headerString = header.toUtf8();
        headerString += ": ";
        headerString += value;
        headerString.append(char(0));

        _curlHttpHeaders = curl_slist_append(_curlHttpHeaders, headerString.constData());
    }

    set(CURLOPT_HTTPHEADER, _curlHttpHeaders);
}

void CurlEasy::setReadFunction(const CurlEasy::ReadFunction& function)
{
    _readFunction = function;
    if (_readFunction) {
        set(CURLOPT_READFUNCTION, staticCurlReadFunction);
        set(CURLOPT_READDATA, this);
    } else {
        set(CURLOPT_READFUNCTION, nullptr);
        set(CURLOPT_READDATA, nullptr);
    }
}

void CurlEasy::setWriteFunction(const CurlEasy::WriteFunction& function)
{
    _writeFunction = function;
    if (_writeFunction) {
        set(CURLOPT_WRITEFUNCTION, staticCurlWriteFunction);
        set(CURLOPT_WRITEDATA, this);
    } else {
        set(CURLOPT_WRITEFUNCTION, nullptr);
        set(CURLOPT_WRITEDATA, nullptr);
    }
}

void CurlEasy::setHeaderFunction(const CurlEasy::HeaderFunction& function)
{
    _headerFunction = function;
    if (_headerFunction) {
        set(CURLOPT_HEADERFUNCTION, staticCurlHeaderFunction);
        set(CURLOPT_HEADERDATA, this);
    } else {
        set(CURLOPT_HEADERFUNCTION, nullptr);
        set(CURLOPT_HEADERDATA, nullptr);
    }
}

void CurlEasy::setSeekFunction(const CurlEasy::SeekFunction& function)
{
    _seekFunction = function;
    if (_seekFunction) {
        set(CURLOPT_SEEKFUNCTION, staticCurlSeekFunction);
        set(CURLOPT_SEEKDATA, this);
    } else {
        set(CURLOPT_SEEKFUNCTION, nullptr);
        set(CURLOPT_SEEKDATA, nullptr);
    }
}

size_t CurlEasy::staticCurlWriteFunction(const void* data, size_t size, size_t nitems, void* easyPtr)
{
    CurlEasy* easy = static_cast<CurlEasy*>(easyPtr);
    assert(easy != nullptr);

    if (easy->_writeFunction) {
        return easy->_writeFunction(data, size * nitems);
    } else {
        return size * nitems;
    }
}

size_t CurlEasy::staticCurlHeaderFunction(const void* data, size_t size, size_t nitems, void* easyPtr)
{
    CurlEasy* easy = static_cast<CurlEasy*>(easyPtr);
    assert(easy != nullptr);

    if (easy->_headerFunction) {
        return easy->_headerFunction(data, size * nitems);
    } else {
        return size * nitems;
    }
}

int CurlEasy::staticCurlSeekFunction(void* easyPtr, curl_off_t offset, int origin)
{
    CurlEasy* easy = static_cast<CurlEasy*>(easyPtr);
    assert(easy != nullptr);

    if (easy->_seekFunction) {
        return easy->_seekFunction(offset, origin);
    } else {
        return CURL_SEEKFUNC_CANTSEEK;
    }
}

size_t CurlEasy::staticCurlReadFunction(void* buffer, size_t size, size_t nitems, void* easyPtr)
{
    CurlEasy* transfer = static_cast<CurlEasy*>(easyPtr);
    assert(transfer != nullptr);

    if (transfer->_readFunction) {
        return transfer->_readFunction(buffer, size * nitems);
    } else {
        return size * nitems;
    }
}

int CurlEasy::staticCurlXferInfoFunction(void* easyPtr, curl_off_t downloadTotal, curl_off_t downloadNow, curl_off_t uploadTotal, curl_off_t uploadNow)
{
    CurlEasy* transfer = static_cast<CurlEasy*>(easyPtr);
    assert(transfer != nullptr);

    emit transfer->progress(downloadTotal, downloadNow, uploadTotal, uploadNow);
    return 0;
}

bool CurlEasy::removeHttpHeader(const QString& header)
{
    auto numErased = _httpHeaders.erase(header);
    _httpHeadersWereSet = true;
    return numErased != 0;
}

bmcl::Option<const QByteArray&> CurlEasy::findHttpHeaderRaw(const QString& header) const
{
    auto it = _httpHeaders.find(header);
    if (it == _httpHeaders.end()) {
        return bmcl::None;
    }
    return it->second;
}

void CurlEasy::setHttpHeaderRaw(const QString& header, const QByteArray& encodedValue)
{
    _httpHeaders[header] = encodedValue;
    _httpHeadersWereSet = true;
}

bool CurlEasy::set(CURLoption option, const QString& parameter)
{
    return set(option, parameter.toUtf8().constData());
}

bool CurlEasy::set(CURLoption option, const QUrl& parameter)
{
    return set(option, parameter.toEncoded().constData());
}

void CurlEasy::setHttpHeader(const QString& header, const QString& value)
{
    setHttpHeaderRaw(header, QUrl::toPercentEncoding(value));
}

bmcl::Option<QString> CurlEasy::findHttpHeader(const QString& header) const
{
    auto it = _httpHeaders.find(header);
    if (it == _httpHeaders.end()) {
        return bmcl::None;
    }
    return QUrl::fromPercentEncoding(it->second);
}

bool CurlEasy::hasHttpHeader(const QString& header) const
{
    return _httpHeaders.find(header) != _httpHeaders.end();
}

bool CurlEasy::isRunning() const
{
    return _isRunning;
}

CURLcode CurlEasy::result() const
{
    return _lastResult;
}

CURL* CurlEasy::handle()
{
    return _handle;
}
}
