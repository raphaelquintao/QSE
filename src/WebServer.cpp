#include "WebServer.h"

#include <sstream>
#include <fstream>

#include <boost/network/uri/uri.hpp>

using namespace boost::network;

//struct HTTP_Handler;
//typedef boost::network::http::server<HTTP_Handler> HTTP_Server;

struct WebServer::HTTP_Handler {
    Searcher *searcher;
    std::ifstream file_reader;

    std::string readFile(const std::string &path, bool &found) {
        file_reader.open("www" + path, std::ifstream::binary);
        if (file_reader.is_open()) {
            std::stringstream temp;
            temp << file_reader.rdbuf();
            file_reader.close();
            found = true;
            return temp.str();
        }
        return "{\"error\":\"invalid url\"}";
    }


    void map_to_dir(const std::string &path, HTTP_Server::response &response) {
        HTTP_Server::response_header content_header;
        content_header.name = "Content-Type";
        content_header.value = "text/html";

        HTTP_Server::response_header control_origin_header;
        control_origin_header.name = "Access-Control-Allow-Origin";
        control_origin_header.value = "*";

        response.status = HTTP_Server::response::ok;
        response.headers.push_back(control_origin_header);

        bool found = false;
        response.content = readFile(path, found);

        if (found) {

        } else {
//            response.headers.push_back(content_header);
            response = HTTP_Server::response::stock_reply(HTTP_Server::response::not_found, "Page Not Found");
        }


    }

    void parse_query(uri::uri &qUri, HTTP_Server::response &response) {
        std::map<std::string, std::string> map;
        uri::query_map(qUri, map);

        HTTP_Server::response_header content_header;
        content_header.name = "Content-Type";
        content_header.value = "application/json";

        HTTP_Server::response_header control_origin_header;
        control_origin_header.name = "Access-Control-Allow-Origin";
        control_origin_header.value = "*";

        response.status = HTTP_Server::response::ok;
        response.headers.push_back(content_header);
        response.headers.push_back(control_origin_header);
        response.content = "{\"error\":\"invalid url\"}";

        if (map.size() == 2 || map.size() == 3) {
            std::string query, model;
            int trunc = 0;
            bool ok = true;
            for (auto i = map.begin(); i != map.end(); ++i) {
                if (i->first == "query") query = i->second;
                else if (i->first == "model") model = i->second;
                else if (i->first == "trunc") trunc = atoi(i->second.c_str());
                else ok = false;
            }
            if (ok) {
                response.content = searcher->Search(query, model, trunc);
            }
        }

    }

    std::string GetHeader(const std::string &name, HTTP_Server::request const &request) {
        for (const auto &header : request.headers) {
            if (header.name == name) return header.value;
        }
        return "";
    }

    void operator()(HTTP_Server::request const &request, HTTP_Server::response &response) {

        std::stringstream full_url;
        full_url << "http://" << request.source << request.destination;

        uri::uri qUri(full_url.str());

        std::string path = qUri.path();
        if (qUri.path() == "/search.json") {
            parse_query(qUri, response);
            std::cout << "  Access -> HOST: " << GetHeader("Host", request) << " REQ: " << uri::decoded_query(qUri) << std::endl;
            return;
        } else if (qUri.path() == "/") {
            path = "/index.html";
        } else if (qUri.path() == "/index.c++") {
            path = "/index.html";
        }
        map_to_dir(path, response);


//        response = HTTP_Server::response::stock_reply(HTTP_Server::response::ok, response_content);
    }

    void log(HTTP_Server::string_type const &info) {
        std::cerr << "ERROR: " << info << '\n';
    }

    HTTP_Handler(Searcher *searcher) {
        this->searcher = searcher;
    }


};


WebServer::WebServer(Searcher *qSearcher) {
    this->qSearcher = qSearcher;

}

void WebServer::Start(std::string port) {

    HTTP_Handler handler(qSearcher);

    HTTP_Server::options options(handler);

    HTTP_Server server(options.address("::").port(port));

    printf(" Web Server - Starting on port %s ! \n", port.c_str());

    server.run();


}