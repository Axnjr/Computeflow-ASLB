#define DEV_MODE

#include "dependencies/httplib.h"
#include "ip_cache.h"
#include "load_config.h"
#include "utils.h"
#include "rapidjson/document.h"
#include "lb_config_struct.h"
//#include "scaling.h"
#include "logger.h"
#include "notifications.h"
#include <iostream>
#include <mutex>

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

static int CURRENT_IP_POOL_INDEX = 0;

int ip_cache::cap = 100;

static pair<int64_t, std::chrono::system_clock::time_point> REQUESTS = { 
	0, 
	chrono::system_clock::now() 
};

list<string> ip_cache::lru;

unordered_map<string, pair<list<string>::iterator, int>> ip_cache::lru_map;
unordered_map<string, CLIENT_USAGE_DATA> client_usage_map;

static string map_clientIp_to_serverIp(string ip) {

	int r = cache.find_ip(ip, CURRENT_IP_POOL_INDEX);

	if (r != -1) {
		return LB_CONFIG::IP_POOL[r];
	}

	else if (CURRENT_IP_POOL_INDEX >= LB_CONFIG::IP_POOL.size()) {
		CURRENT_IP_POOL_INDEX = 0;
		//cout << "REQUEST COUNT RESET !!" << endl;
	}

	//cout << "REQUEST COUNT: " << HTTP_REQUEST_COUNT << endl;
	return LB_CONFIG::IP_POOL[CURRENT_IP_POOL_INDEX++];
}

static void request_handler(const Request& req, Response& res) {
	string ip = map_clientIp_to_serverIp(req.remote_addr);
	httplib::Client PROXY_CLIENT("172.27.48.1", 3000); // port to be added
	auto op = PROXY_CLIENT.Get(req.path);
	cout << "IP:" << ip << op.error() << endl;
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
		//thread(scale_up).detach();
		notify_via_email(
			"Traffic Increased !!",

			"The load-balancer added a VM to the fleet due to incresed traffic, at: " + 
			timePointToString(chrono::system_clock::now()) +
			"\n CPU UASGE: " +
			to_string(usage_data.cpuUsage) +
			"\n MEMORY CONSUMPTION: " + 
			to_string(usage_data.memUsage)
		);
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
		cout << endl << usage_data.cpuUsage << " | " << usage_data.memUsage << endl;
		// scale vm down ..
		//thread(scale_down, usage_data.ip).detach();
		notify_via_email(
			"Traffic Decreased !!",

			"The load-balancer removed a VM from the fleet due to decresed traffic, at: " +
			timePointToString(chrono::system_clock::now()) +
			"\n CPU UASGE: " +
			to_string(usage_data.cpuUsage) +
			"\n MEMORY CONSUMPTION: " +
			to_string(usage_data.memUsage)
		);
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

	svr.set_pre_routing_handler([](const auto& req, auto& res) {
		REQUESTS.first++;
	    return Server::HandlerResponse::Unhandled;
	});

	svr.Post(".*", [&](const Request& req, Response& res)
	{
		if (req.path == "/ping")
		{
			// Request count is reset every 15 mins
			REQUESTS.first = 0;
			REQUESTS.second = chrono::system_clock::now();

			ltf("PING FROM VM: ", req.remote_addr, " STATUS: ", req.body);

			JsonMessage.Parse(req.body.c_str());
			if (JsonMessage.HasParseError()) {
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

		else if (req.path == "/ping_error") 
		{
			// Request count is reset every 15 mins
			REQUESTS.first = 0;
			REQUESTS.second = chrono::system_clock::now();

			ltf("PING ERROR OCCURED: ", JsonMessage.GetParseError());
		}

		else {
			request_handler(req, res);
		}
	});

	svr.Get(".*", [](const Request& req, Response& res) 
	{
		//thread(notify_via_email, "ASLB TEST EMAIL!", "CHAL GYA! RADHA KANHA.").detach();

			

		#ifdef DEV_MODE
		if (req.path == "/favicon.ico") {
			res.set_content("404", "text/plain");
			return;
		}
		#endif																					// DEV_MODE

		if (req.path == "/status/status") {														// for getting aslb status
			res.set_content(getLBConfigAsJson(REQUESTS.first, timePointToString(REQUESTS.second)), "text/json");
			return;
		}

		else if (req.path == "/logs/logs")														// for getting log file content
		{
			ifstream config_file("aslb.log");
			if (!config_file.is_open()) {
				ltf("Unable to open `aslb_config.json`");
				res.set_content("UNABLE TO LOAD LOG FILE !", "text/plain");
				return;
			}

			string json((istreambuf_iterator<char>(config_file)), istreambuf_iterator<char>());
			res.set_content(json.c_str(), "text/plain");
			return;
		}

		else {
			request_handler(req, res);
			return;
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
	svr.listen("localhost", 4000);

	return 0;
}