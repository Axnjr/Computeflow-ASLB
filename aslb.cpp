#include "headers/ip_cache.h"
#include "headers/load_config.h"
#include "headers/fetch_response.h"
#include "crow.h"
#include <iostream>
#include <vector>

using namespace std;

// Initialize static members

ip_cache cache;

list<string> ip_cache::lru;
unordered_map<string, pair<list<string>::iterator, int>> ip_cache::lru_map;
int ip_cache::cap = 0;

static int REQUEST_COUNT = 0;
static int PORT;
static vector<string> IP_POOL;

// Sticky Session to be implmented
static string get_server_ip(string ip) {
	int r = cache.find_ip(ip, REQUEST_COUNT);
	if (r != -1) {
		return IP_POOL[r];
	}
	else if (REQUEST_COUNT >= IP_POOL.size()) {
		REQUEST_COUNT = 0;
		cout << "REQUEST COUNT RESET !!" << endl;
	}
	cout << "REQUEST COUNT: " << REQUEST_COUNT << endl;
	return IP_POOL[REQUEST_COUNT++];
}

int main() {

	crow::SimpleApp Server;

	initialize_static_memory_from_config(PORT, IP_POOL);

	CROW_CATCHALL_ROUTE(Server)([](const crow::request& req, crow::response& res) {

		string ip = get_server_ip(req.remote_ip_address);
		auto op = fetch_response_from_ip("", req.raw_url, PORT);
		
		if (op) {
			res.code = op->status;
			res.body = op->body;
			if (req.raw_url != "/") {
				for (const auto& header : op->headers) {
					//res.add_header(header.first, header.second);
					res.headers.emplace(header.first, header.second);
				}
			}
			// All headers set in the response are being logged correctly but they are present when checked in the networks tab in dev tools 
			// and file contents are missing. I recive 9 headers from original server but only 4 are added to current headers 😢
			std::cout << "Headers set in response:" << std::endl;
			for (const auto& header : res.headers) {
				std::cout << header.first << ": " << header.second << std::endl;
			}
		}

		else {
			res.code = 502; // Bad Gateway
			res.write("Failed to forward request: Loadbalancer Error");
		}

		res.set_header("BODY", op->body.substr(0, 100));
		//cout << endl << "BODY: === " << endl << op->body << endl;

		//res.write(ip);
		//cache.show_cache(IP_POOL);
		res.end();
	});

	Server.loglevel(crow::LogLevel::Info);
	Server.port(4000).multithreaded().run();

	return 0;
}

//// Ensure correct content type for JavaScript files
//			//res.set_header("Content-Type", PROXY_RES->get_header_value("Content-Type"));
//			/*if (req.raw_url.find(".js") != string::npos) {
//				res.set_header("Content-Type", "application/javascript; charset=utf-8");
//				cout << endl << "//////////// .JS CONTENT TYPE ///////////////////////////////////" << endl << endl;
//			}
//			else if (req.raw_url.find(".css") != string::npos) {
//				res.set_header("Content-Type", "text/css");
//				cout << endl << "CCCCCCCCCCCCCCCCCC .CSS CONTENT TYPE CCCCCCCCCCCCCCCCCCCCCCC" << endl << endl;
//			}
//			else {
//				string content_type = get_mime_type(req.raw_url);
//				res.set_header("Content-Type", content_type);
//				cout << endl << "============= .ELSE CONTENT TYPE ==================" << content_type << endl << endl;
//			}*/
//			//cout << endl << "----------------------------- START ---------------------------------" << endl;
////cout << req.raw_url << endl;
////cout << "Header Key:" << "___________" << "Header Value:" << endl;
//	/*i = 1;
//			cout << endl << endl << "MODIFIED HEADERS !!!!! :::::: ?????? >>>>>> <<<<< {{{{{{}}}}}}" << endl << endl;
//			for (auto u : res.headers) {
//				cout << endl << endl << i << "th ITERATION" << endl << endl << endl;
//				cout << u.first << ";;;" << u.second << endl;
//				i++;
//			}*/
//			//cout << "------------------------------- END ----------------------------------" << endl << endl;
//			//auto it = op->headers.begin();
//			//for (; it != op->headers.end(); ++it) {
//			//	const auto& header = *it;
//			//	std::cout << "\n(" << header.first << "<=============>" << header.second << ")\n";
//			//	if (header.first == "Content-Type") {  // Skip chunked encoding
//			//		res.set_header(header.first, header.second);
//			//	}
//			//}
//			//res.set_header("Content-Type", op->headers.find())
//			//res.set_header("HOST-4000", "PROXY ACTIVE !!");
//			//cout << "========================================= END ==========================================" << endl;
///*cout << "|---------------------------------------------------------------------|" << endl;
//		cout << "|			RAW URL:" << "$-->" << req.raw_url << "<--$				   |" << endl;
//		cout << "|			URL PARAMS:" << req.url_params << "						   |" << endl;
//		cout << "|---------------------------------------------------------------------|" << endl << endl;*/



//#include "headers/ip_cache.h"
//#include "headers/load_config.h"
//#include "headers/fetch_response.h"
//#include "crow.h"
//#include <iostream>
//#include <vector>
//
//using namespace std;

// Initialize static members

//ip_cache cache;
//
//list<string> ip_cache::lru;
//unordered_map<string, pair<list<string>::iterator, int>> ip_cache::lru_map;
//int ip_cache::cap = 0;

//static int REQUEST_COUNT = 0;
//static int PORT;
//static vector<string> IP_POOL;

// Sticky Session to be implmented
//static string get_server_ip(string ip) {
//	int r = cache.find_ip(ip, REQUEST_COUNT);
//	if (r != -1) {
//		return IP_POOL[r];
//	}
//	else if (REQUEST_COUNT >= IP_POOL.size()) {
//		REQUEST_COUNT = 0;
//		cout << "REQUEST COUNT RESET !!" << endl;
//	}
//	cout << "REQUEST COUNT: " << REQUEST_COUNT << endl;
//	return IP_POOL[REQUEST_COUNT++];
//}

//#include "httplib.h"
//#include "crow.h"
//#include <iostream>
//#include <vector>
//
//using namespace std;
//using namespace httplib;
//
//int main() {
//	crow::SimpleApp Server;
//	CROW_CATCHALL_ROUTE(Server)([](const crow::request& req, crow::response& res) {
//		httplib::Client PROXY_CLIENT("172.27.48.1", 3000); // port to be added
//		auto original_response = PROXY_CLIENT.Get(req);
//		if (original_response) {
//			res.code = original_response->status;
//			res.body = original_response->body;
//			if (req.raw_url != "/") {
//				cout << "ORIGINAL HEADERS:" << endl;
//				for (const auto& header : original_response->headers) { // 
//					cout << "(" << header.first << " , " << header.second << ")" << endl;
//					res.set_header(header.first, header.second);
//				}
//			}
//			// All headers set in the response are being logged correctly but they are present when checked in the networks tab in dev tools 
//			// and file contents are missing. I recive 9 headers from original server but only 4 are added to current headers 😢
//			std::cout << "Headers set in response:" << std::endl;
//			for (const auto& header : res.headers) {
//				std::cout << header.first << ": " << header.second << std::endl;
//			}
//		}
//		else {
//			res.code = 502; // Bad Gateway
//			res.write("Failed to forward request: Loadbalancer Error");
//		}
//		res.set_header("BODY", original_response->body.substr(0, 100)); // just to check
//		res.end();
//		});
//	Server.loglevel(crow::LogLevel::Info);
//	Server.port(4000).multithreaded().run();
//	return 0;
//}