// based on public domain code https://github.com/tarasvb/qtcurl.git

#pragma once

#include "mcc/Config.h"

#include <QObject>

#include <curl/curl.h>

#include <cstddef>
#include <vector>

class QTimer;

namespace mccmap {

struct CurlMultiSocket;
class CurlEasy;

class MCC_MAP_DECLSPEC CurlMulti : public QObject {
    Q_OBJECT
public:
    explicit CurlMulti(QObject* parent = nullptr);
    virtual ~CurlMulti();

    //NOTE: CurlMulti ownes easy handles
    CurlEasy* addTransfer();

private slots:
    void curlMultiTimeout();
    void socketReadyRead(int socketDescriptor);
    void socketReadyWrite(int socketDescriptor);
    void socketException(int socketDescriptor);

private:
    friend class CurlEasy;

    void onEasyDelete(CurlEasy* transfer);
    void startEasy(CurlEasy* transfer);
    void stopEasy(CurlEasy* transfer);

    void curlSocketAction(curl_socket_t socketDescriptor, int eventsBitmask);
    int curlTimerFunction(int timeoutMsec);
    int curlSocketFunction(CURL* easyHandle, curl_socket_t socketDescriptor, int action, CurlMultiSocket* socket);
    static int staticCurlTimerFunction(CURLM* multiHandle, long timeoutMs, void* userp);
    static int staticCurlSocketFunction(CURL* easyHandle, curl_socket_t socketDescriptor, int what, void* userp, void* sockp);

    QTimer* _timer;
    CURLM* _handle;

    std::vector<CurlEasy*> _transfers;
};
}
