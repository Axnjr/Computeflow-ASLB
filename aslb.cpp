#define DEV_MODE
#include "dependencies/httplib.h"
#include "ip_cache.h"
#include "load_config.h"
#include "fetch_response.h"
#include "utils.h"

#include "dependencies/easywsclient.hpp"
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif

using namespace std;
using namespace httplib;
using easywsclient::WebSocket;

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
	std::unique_ptr<WebSocket> ws(WebSocket::from_url("ws://localhost:3000/"));
	//assert(ws);

	if (!ws) {
		cout << endl << "Could not connect with VM: " << ip << endl;
		// handle connection error / vm might be down
		return;
	}

	cout << "TRYING !!" << endl;

	//#ifdef DEV_MODE
		ws->send("JOIN");
		cout << "JOIN SIGNAL SENT" << endl;
	//#endif // DEV_MODE
		//ws->send("PING ASLB");

	while (ws->getReadyState() != WebSocket::CLOSED) {
		ws->poll();
		ws->dispatch([](const std::string& message) {
			printf(">>> %s\n", message.c_str());
		});
	}
}

int main(void) {

	initialize_static_memory_from_config(PORT, IP_POOL);

	Server svr;
	svr.set_logger([](const Request& req, const Response& res) { cout << log(req, res); });

	svr.Get(".*", [](const Request& req, Response& res) {
		//cout << endl << "CLIENT-IP: " << req.remote_addr << endl;
		string ip = map_clientIp_to_serverIp(req.remote_addr);
		auto op = fetch_response_from_ip("", req.path, PORT);
		res.body = op->body;
		res.status = op->status;
		res.headers = op->headers;
	});

	thread([]() {
		cout << endl << "WEBSOCKET THREAD STARTED !" << endl;
		cout << endl << "Thread Id:" << this_thread::get_id() << endl;
		while (true) {
			cout << "IN THE LOOP" << endl;

			if (WS_REQUEST_COUNT >= IP_POOL.size()) {
				WS_REQUEST_COUNT = 0;
			}

			fetch_vm_info(IP_POOL[WS_REQUEST_COUNT]);

			this_thread::sleep_for(0.3s);
			WS_REQUEST_COUNT++;
		}
	}).detach();

	cout << "SERVER STARTED ON PORT: [4000]" << endl;
	svr.listen("localhost", 4000);

	return 0;
}