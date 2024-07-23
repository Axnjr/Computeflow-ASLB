#include "httplib.h"
#include "rapidjson/document.h"
#include "crow_include/crow.h"
#include <iostream>

using namespace std;
using namespace httplib;

int main() {
    crow::SimpleApp Server;

    CROW_CATCHALL_ROUTE(Server)([](const crow::request& req, crow::response& res, std::string path) {
        std::string host = req.get_header_value("Host");
        std::string subdomain = host.substr(0, host.find('.'));

        cout << "HOST:" << host << endl;
        cout << "SUB DOMAIN:" << subdomain << endl;
        cout << "BODY:" << req.body << endl;

        cout << "PATH:" << endl;
        for (auto i : path) {
            cout << i << endl;
        }

        res.write("WRITTING A RESPONSE TO `write` method !");
        res.end();
        });

    Server.loglevel(crow::LogLevel::Info);
    Server.port(3000).multithreaded().run_async();
    return 0;
}