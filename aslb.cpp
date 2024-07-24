#include "headers/load_balancer.h"
#include "headers/load_config.h"
#include "headers/fetch_response.h"
#include "crow.h"
#include <iostream>
#include <vector>

using namespace std;

int main() {
	crow::SimpleApp Server;

	static int REQUEST_COUNT = 0;
	static int PORT;
	static vector<string> IP_POOL;

	initialize_static_memory_from_config(PORT, IP_POOL);

	CROW_CATCHALL_ROUTE(Server)([](const crow::request& req, crow::response& res) {
		cout << "RAW URL:" << req.raw_url << endl;
		cout << "REMOTE IP ADDRESS:" << req.remote_ip_address << endl;
		cout << "URL PARAMS:" << req.url_params << endl;
		string ip = get_server_ip(REQUEST_COUNT, IP_POOL);
		auto PROXY_RES = fetch_response_from_ip(ip, req.raw_url , PORT);
		if (PROXY_RES) {
			res.code = PROXY_RES->status;
			res.body = PROXY_RES->body;
			res.set_header("Content-Type", PROXY_RES->get_header_value("Content-Type"));
		}
		else {
			res.code = 502; // Bad Gateway
			res.write("Failed to forward request: Loadbalancer Error");
		}
		res.write(ip);
		res.end();
	});

	Server.loglevel(crow::LogLevel::Info);
	Server.port(3000).multithreaded().run();
	return 0;
}