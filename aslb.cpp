#include "headers/httplib.h"
#include "crow.h"
#include "headers/load_balancer.h"
#include <iostream>
#include <vector>

using namespace std;
//using namespace httplib;

int main() {
	crow::SimpleApp Server;
	static int REQUEST_COUNT = 0;

   CROW_CATCHALL_ROUTE(Server)([](const crow::request& req, crow::response& res) {
		cout << "RAW URL:" << req.raw_url << endl;
		cout << "REMOTE IP ADDRESS:" << req.remote_ip_address << endl;
		cout << "URL PARAMS:" << req.url_params << endl;
		string ip = get_server_ip(REQUEST_COUNT);
		res.write(ip);
		res.end();
   });

	Server.loglevel(crow::LogLevel::Info);
	Server.port(3000).multithreaded().run();
	return 0;
}