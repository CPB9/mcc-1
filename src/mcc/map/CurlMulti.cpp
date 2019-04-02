// based on public domain code https://github.com/tarasvb/qtcurl.git

// This disables min & max macros declaration in windows.h which will be included somewhere there
#define NOMINMAX

#include "mcc/map/CurlMulti.h"
#include "mcc/map/CurlEasy.h"

#include <bmcl/Logging.h>

#include <QTimer>
#include <QSocketNotifier>

#include <limits>
#include <memory>
#include <cassert>

namespace mccmap {

struct CurlMultiSocket {
    CurlMultiSocket(curl_socket_t socket, QObject* parent = nullptr)
        : socketDescriptor(socket)
        , readNotifier(socket, QSocketNotifier::Read, parent)
        , writeNotifier(socket, QSocketNotifier::Write, parent)
        , errorNotifier(socket, QSocketNotifier::Exception, parent)
    {
    }

    curl_socket_t socketDescriptor;
    QSocketNotifier readNotifier;
    QSocketNotifier writeNotifier;
    QSocketNotifier errorNotifier;
};

CurlMulti::CurlMulti(QObject* parent)
    : QObject(parent)
    , _timer(new QTimer(this))
    , _handle(curl_multi_init())
{
    assert(_handle != nullptr);

    curl_multi_setopt(_handle, CURLMOPT_SOCKETFUNCTION, staticCurlSocketFunction);
    curl_multi_setopt(_handle, CURLMOPT_SOCKETDATA, this);
    curl_multi_setopt(_handle, CURLMOPT_TIMERFUNCTION, staticCurlTimerFunction);
    curl_multi_setopt(_handle, CURLMOPT_TIMERDATA, this);

    _timer->setSingleShot(true);
    connect(_timer, &QTimer::timeout, this, &CurlMulti::curlMultiTimeout);
}

CurlMulti::~CurlMulti()
{
    auto transfers = std::move(_transfers);

    for (CurlEasy* easy : transfers) {
        stopEasy(easy);
        delete easy;
    }

    curl_multi_cleanup(_handle);
}

void CurlMulti::startEasy(CurlEasy* easy)
{
    curl_multi_add_handle(_handle, easy->handle());
}

void CurlMulti::stopEasy(CurlEasy* easy)
{
    curl_multi_remove_handle(_handle, easy->handle());
}

CurlEasy* CurlMulti::addTransfer()
{
    CurlEasy* easy = new CurlEasy(this);
    _transfers.push_back(easy);
    return easy;
}

void CurlMulti::onEasyDelete(CurlEasy* easy)
{
    stopEasy(easy);
    auto begin = std::remove(_transfers.begin(), _transfers.end(), easy);
    _transfers.erase(begin, _transfers.end());
}

//TODO: exception notifier
int CurlMulti::curlSocketFunction(CURL* easyHandle, curl_socket_t socketDescriptor, int action, CurlMultiSocket* socket)
{
    Q_UNUSED(easyHandle);
    if (!socket) {
        if (action == CURL_POLL_REMOVE || action == CURL_POLL_NONE) {
            return 0;
        }

        socket = new CurlMultiSocket(socketDescriptor, this);
        connect(&socket->readNotifier, &QSocketNotifier::activated, this, &CurlMulti::socketReadyRead);
        connect(&socket->writeNotifier, &QSocketNotifier::activated, this, &CurlMulti::socketReadyWrite);
        connect(&socket->errorNotifier, &QSocketNotifier::activated, this, &CurlMulti::socketException);
        socket->errorNotifier.setEnabled(true);
        curl_multi_assign(_handle, socketDescriptor, socket);
    }

    if (action == CURL_POLL_REMOVE) {
        curl_multi_assign(_handle, socketDescriptor, nullptr);

        // Note: deleteLater will NOT work here since there are
        //       situations where curl subscribes same sockect descriptor
        //       until events processing is done and actual delete happen.
        //       This causes QSocketNotifier not to register notifications again.
        delete socket;
        return 0;
    }

    socket->readNotifier.setEnabled(action == CURL_POLL_IN || action == CURL_POLL_INOUT);
    socket->writeNotifier.setEnabled(action == CURL_POLL_OUT || action == CURL_POLL_INOUT);

    return 0;
}

int CurlMulti::curlTimerFunction(int timeoutMsec)
{
    if (timeoutMsec >= 0) {
        _timer->start(timeoutMsec);
    } else {
        _timer->stop();
    }

    return 0;
}

void CurlMulti::curlMultiTimeout()
{
    curlSocketAction(CURL_SOCKET_TIMEOUT, 0);
}

void CurlMulti::socketReadyRead(int socketDescriptor)
{
    curlSocketAction(socketDescriptor, CURL_CSELECT_IN);
}

void CurlMulti::socketReadyWrite(int socketDescriptor)
{
    curlSocketAction(socketDescriptor, CURL_CSELECT_OUT);
}

void CurlMulti::socketException(int socketDescriptor)
{
    curlSocketAction(socketDescriptor, CURL_CSELECT_ERR);
}

void CurlMulti::curlSocketAction(curl_socket_t socketDescriptor, int eventsBitmask)
{
    int runningHandles;

    CURLMcode rc = curl_multi_socket_action(_handle, socketDescriptor, eventsBitmask, &runningHandles);

#if (LIBCURL_VERSION_MAJOR < 7) || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR < 20)
    while (rc == CURLM_CALL_MULTI_PERFORM) {
        rc = curl_multi_socket_action(_handle, socketDescriptor, eventsBitmask, &runningHandles);
    }
#endif

    if (rc != CURLM_OK) {
        BMCL_CRITICAL() << "curl multi error: " << curl_multi_strerror(rc);
        // TODO: Handle global curl errors
    }

    int messagesLeft = 0;
    do {
        CURLMsg* message = curl_multi_info_read(_handle, &messagesLeft);

        if (message == nullptr) {
            break;
        }

        if (message->easy_handle == nullptr) {
            continue;
        }

        CurlEasy* transfer = nullptr;
        curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, &transfer);

        if (transfer == nullptr) {
            continue;
        }

        transfer->onCurlMessage(message);
    } while (messagesLeft);
}

int CurlMulti::staticCurlSocketFunction(CURL* easyHandle, curl_socket_t socketDescriptor, int what, void* userp, void* sockp)
{
    CurlMulti* multi = static_cast<CurlMulti*>(userp);
    assert(multi != nullptr);

    return multi->curlSocketFunction(easyHandle, socketDescriptor, what, static_cast<CurlMultiSocket*>(sockp));
}

int CurlMulti::staticCurlTimerFunction(CURLM* multiHandle, long timeoutMs, void* userp)
{
    (void)multiHandle;
    CurlMulti* multi = static_cast<CurlMulti*>(userp);
    assert(multi != nullptr);

    int intTimeoutMs;

    if (timeoutMs >= std::numeric_limits<int>::max()) {
        intTimeoutMs = std::numeric_limits<int>::max();
    } else if (timeoutMs >= 0) {
        intTimeoutMs = static_cast<int>(timeoutMs);
    } else {
        intTimeoutMs = -1;
    }

    return multi->curlTimerFunction(intTimeoutMs);
}
}
