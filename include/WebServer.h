#ifndef QSE_WEBSERVER_H
#define QSE_WEBSERVER_H

#include "Searcher.h"
#include <string>
#include <boost/network/protocol/http/server.hpp>

class WebServer {
    struct HTTP_Handler;
    typedef boost::network::http::server<HTTP_Handler> HTTP_Server;

    Searcher *qSearcher;

public:
    WebServer(Searcher *qSearcher);

    void Start(std::string port);
};


#endif //QSE_WEBSERVER_H
