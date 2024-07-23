// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
#include "httplib.h"
#include "rapidjson/document.h"
#include "crow_include/crow.h"
#include <iostream>
using namespace std;
using namespace httplib;

int main(void) {
    Server http;
    int port = 3000;

    /*#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        SSLServer https(SERVER_CERT_FILE, SERVER_PRIVATE_KEY_FILE);
    #endif*/

    http.set_logger([](const httplib::Request& req, const httplib::Response& res) {
        cout << req.method << "🎊" << req.path << "🎉" << res.status << endl;
        });

    http.Get("/", [](const Request& req, Response& res) {
        string host = req.get_header_value("Host");
        string subdomain = host.substr(0, host.find('.'));
        cout << "Compiling Route: GET[/]" << endl;
        cout << "LOCAL ADDRESS: " << req.local_addr << endl;
        cout << "Host: " << host << endl;
        cout << "Subdomain: " << subdomain << endl;
        res.set_content("Test\n", "text/plain");
        });

    http.Get("/error", [](const Request& req, Response& res) {
        res.set_content("Some Unexpected Error Occurred !", "text/plain");
        });

    http.set_error_handler([](const Request& /*req*/, Response& res) {
        res.set_redirect("http://localhost:3000/error");
        });

    //#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    //    https.Get("/", [=](const Request& /*req*/, Response& res) {
    //        res.set_redirect("/hi");
    //        });
    //    https.Get("/hi", [](const Request& /*req*/, Response& res) {
    //        res.set_content("Hello World!\n", "text/plain");
    //        });
    //    https.Get("/stop", [&](const Request& /*req*/, Response& /*res*/) {
    //        https.stop();
    //        http.stop();
    //        });
    //#endif

    auto httpThread = std::thread([&]() {
        http.listen("localhost", port);
        cout << "The server started at port " << port << "..." << endl;
        });

    //#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    //    auto httpsThread = std::thread([&]() { https.listen("localhost", 8081); });
    //#endif

    httpThread.join();

    //#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    //    httpsThread.join();
    //#endif

    return 0;
}
