#define DEV_MODE
#include "dependencies/httplib.h"
#include "ip_cache.h"
#include "load_config.h"
#include "fetch_response.h"
#include "utils.h"
#include "rapidjson/document.h"
#include "lb_config_struct.h"
#include "scaling.h"
#include "logger.h"
#include <iostream>

using namespace std;
using namespace httplib;
using namespace logger;

struct CLIENT_USAGE_DATA {

	string ip;
	int cpus;

	float freeMemoryInMB;
	float systemUptime;
	float totalMemory;
	float cpuUsage;
	float memUsage;

	chrono::time_point<chrono::steady_clock> lastUpdated;
};

ip_cache cache;

int ip_cache::cap = 100;
list<string> ip_cache::lru;
unordered_map<string, pair<list<string>::iterator, int>> ip_cache::lru_map;
unordered_map<string, CLIENT_USAGE_DATA> client_usage_map;

static int HTTP_REQUEST_COUNT = 0;

static string map_clientIp_to_serverIp(string ip) {

	int r = cache.find_ip(ip, HTTP_REQUEST_COUNT);

	if (r != -1) {
		return LB_CONFIG::IP_POOL[r];
	}

	else if (HTTP_REQUEST_COUNT >= LB_CONFIG::IP_POOL.size()) {
		HTTP_REQUEST_COUNT = 0;
		//cout << "REQUEST COUNT RESET !!" << endl;
	}

	//cout << "REQUEST COUNT: " << HTTP_REQUEST_COUNT << endl;
	return LB_CONFIG::IP_POOL[HTTP_REQUEST_COUNT++];
}

static void request_handler(const Request& req, Response& res) {
	string ip = map_clientIp_to_serverIp(req.remote_addr);
	auto op = fetch_response_from_ip("", req.path, LB_CONFIG::PORT);
	res.body = op->body;
	res.status = op->status;
	res.headers = op->headers;
}

static void analyze_vm_state(CLIENT_USAGE_DATA usage_data) {

	if(
		(
			usage_data.cpuUsage > LB_CONFIG::max_cpu_usage
								||													// OR
			usage_data.memUsage > LB_CONFIG::max_mem_usage
		)
						&&
		LB_CONFIG::vmCount < LB_CONFIG::maxVms
	)
	{
		// scale vm up ..
		thread(scale_up).detach();
		return;
	}

	if(
		(
			usage_data.cpuUsage < LB_CONFIG::min_cpu_usage
								||													// OR
			usage_data.memUsage < LB_CONFIG::min_mem_usage
		)
						&&
		LB_CONFIG::vmCount > LB_CONFIG::minVms
	)
	{
		// scale vm down ..
		thread(scale_down, usage_data.ip).detach();
		return;
	}

}

int main(void) {

	initialize_static_memory_from_config();

	Server svr;
	Document JsonMessage;

	svr.set_logger([](const Request& req, const Response& res) { 
		cout << log(req, res); 
	});

	svr.Post(".*", [&](const Request& req, Response& res)
	{
		if (req.path == "/ping")
		{
			ltf("PING FROM VM: ", req.remote_addr, " STATUS: ", req.body);

			JsonMessage.Parse(req.body.c_str());
			if (JsonMessage.HasParseError())
			{
				ltf("Error parsing `PING REQUEST PAYLOAD`: ", JsonMessage.GetParseError());
			}

			auto client_usage_map_it = client_usage_map.find(req.remote_addr);

			if (client_usage_map_it == client_usage_map.end())
			{
				CLIENT_USAGE_DATA cud;

				cud.cpus = JsonMessage["cpus"].GetInt();
				cud.cpuUsage = JsonMessage["cpuUsage"].GetFloat();
				cud.memUsage = JsonMessage["memoryUsage"].GetFloat();
				cud.systemUptime = JsonMessage["systemUptime"].GetFloat();
				cud.freeMemoryInMB = JsonMessage["freeMemoryInMB"].GetFloat();
				cud.totalMemory = JsonMessage["totalMemory"].GetFloat();
				cud.ip = req.remote_addr;
				cud.lastUpdated = chrono::steady_clock::now();

				client_usage_map.emplace(req.remote_addr, cud);
				analyze_vm_state(cud);
			}
			else
			{
				auto& temp = *client_usage_map_it;

				temp.second.cpuUsage = JsonMessage["cpuUsage"].GetFloat();
				temp.second.memUsage = JsonMessage["memoryUsage"].GetFloat();
				temp.second.systemUptime = JsonMessage["systemUptime"].GetFloat();
				temp.second.freeMemoryInMB = JsonMessage["freeMemoryInMB"].GetFloat();
				temp.second.totalMemory = JsonMessage["totalMemory"].GetFloat();
				temp.second.lastUpdated = chrono::steady_clock::now();

				analyze_vm_state(temp.second);
			}
			res.status = 200;
		}
		else if (req.path == "/ping_error") {
			ltf("PING ERROR OCCURED: ", JsonMessage.GetParseError());
		}
		else {
			request_handler(req, res);
		}
	});

	svr.Get(".*", [](const Request& req, Response& res) {
		if (req.path == "/status/status") {
			res.set_content(getLBConfigAsJson(), "text/json");
		}
		else if (req.path == "/logs/logs") {
			// logs to be implemented
		}
		else {
			request_handler(req, res);
		}
	});

	svr.Delete(".*", [](const Request& req, Response& res) {
		request_handler(req, res);
	});

	svr.Patch(".*", [](const Request& req, Response& res) {
		request_handler(req, res);
	});

	svr.Put(".*", [](const Request& req, Response& res) {
		request_handler(req, res);
	});

	//ltf("SERVER STARTED ON PORT: [4000]");
	cout << "SERVER STARTED ON PORT: [4000]" << endl;
	svr.listen("2401:4900:1c97:b0f:862:1863:aa28:4766", 4000);

	return 0;
}