#define DEV_MODE
#include "dependencies/httplib.h"
#include "ip_cache.h"
#include "load_config.h"
#include "fetch_response.h"
#include "utils.h"
#include <iostream>
#include "crow.h"

using namespace std;
using namespace httplib;

ip_cache cache;

int ip_cache::cap = 0;
list<string> ip_cache::lru;
unordered_map<string, pair<list<string>::iterator, int>> ip_cache::lru_map;

static int HTTP_REQUEST_COUNT = 0;
static int WS_REQUEST_COUNT = 0;
static int PORT;
static vector<string> IP_POOL;

// Sticky Session to be implmented
static string map_clientIp_to_serverIp(string ip) {
	int r = cache.find_ip(ip, HTTP_REQUEST_COUNT);
	if (r != -1) {
		return IP_POOL[r];
	}
	else if (HTTP_REQUEST_COUNT >= IP_POOL.size()) {
		HTTP_REQUEST_COUNT = 0;
		cout << "REQUEST COUNT RESET !!" << endl;
	}
	cout << "REQUEST COUNT: " << HTTP_REQUEST_COUNT << endl;
	return IP_POOL[HTTP_REQUEST_COUNT++];
}

static void handle_message(const std::string& message) {
	printf(">>> %s\n", message.c_str());
}

static void fetch_vm_info(string ip) {

}

int main(void) {

	initialize_static_memory_from_config(PORT, IP_POOL);

	Server svr;
	crow::SimpleApp WS;

	mutex mtx;
	unordered_set<crow::websocket::connection*> WS_USERS;

	CROW_WEBSOCKET_ROUTE(WS, "/ws")
		.onopen([&](crow::websocket::connection& conn) {
			CROW_LOG_INFO << "new websocket connection from " << conn.get_remote_ip();
			std::lock_guard<std::mutex> _(mtx);
			WS_USERS.insert(&conn);
		})
		.onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
			CROW_LOG_INFO << "websocket connection closed: " << reason;
			std::lock_guard<std::mutex> _(mtx);
			WS_USERS.erase(&conn);
		})
		.onmessage([&](crow::websocket::connection& /*conn*/, const std::string& data, bool is_binary) {
			CROW_LOG_INFO << "RECIVED MESSAGE: " << data;
		});
	
	svr.set_logger([](const Request& req, const Response& res) { cout << log(req, res); });

	svr.Get(".*", [](const Request& req, Response& res) {
		//cout << endl << "CLIENT-IP: " << req.remote_addr << endl;
		string ip = map_clientIp_to_serverIp(req.remote_addr);
		auto op = fetch_response_from_ip("", req.path, PORT);
		res.body = op->body;
		res.status = op->status;
		res.headers = op->headers;
	});

	thread([&]() {
		cout << endl << "WEBSOCKET THREAD / SERVER STARTED ON PORT: [5000] | ThreadId: " << this_thread::get_id() << endl;
		WS.port(5000).run();
	}).detach();

	cout << "SERVER STARTED ON PORT: [4000]" << endl;
	svr.listen("localhost", 4000);

	return 0;
}

//while (true) {
//	cout << "IN THE LOOP" << endl;
//
//	if (WS_REQUEST_COUNT >= IP_POOL.size()) {
//		WS_REQUEST_COUNT = 0;
//	}
//
//	fetch_vm_info(IP_POOL[WS_REQUEST_COUNT]);
//
//	this_thread::sleep_for(0.3s);
//	WS_REQUEST_COUNT++;
//}