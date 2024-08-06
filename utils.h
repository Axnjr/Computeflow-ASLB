#include "dependencies/httplib.h"
#include "logger.h"
#include <chrono>

using namespace std;
using namespace httplib;

static string timePointToString(const chrono::system_clock::time_point& tp) {
    time_t time = chrono::system_clock::to_time_t(tp);
    tm* tm = std::localtime(&time);
    ostringstream oss;
    oss << put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

static string stringify_headers(const Headers& headers) {
    string s;
    char buf[BUFSIZ];
    for (const auto& x : headers) {
        snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
        s += buf;
    }
    return s;
}

static string log(const Request& req, const Response& res) {

    string s;
    string query;
    char buf[BUFSIZ];

    s += "\n===========================================================================================\n";

    snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
    req.version.c_str(), req.path.c_str());
    s += buf;

    for (auto it = req.params.begin(); it != req.params.end(); ++it) {
        const auto& x = *it;
        snprintf(buf, sizeof(buf), "%c%s=%s",
            (it == req.params.begin()) ? '?' : '&', x.first.c_str(),
            x.second.c_str());
        query += buf;
    }

    snprintf(buf, sizeof(buf), "%s\n", query.c_str());
    s += buf;
    s += stringify_headers(req.headers);
    //s += dump_multipart_files(req.files); method removed / disabled

    snprintf(buf, sizeof(buf), "%d\n", res.status);

    s += buf;
    s += stringify_headers(res.headers);
    s += "\n===========================================================================================\n";

    logger::ltf(s);
    return s;
}