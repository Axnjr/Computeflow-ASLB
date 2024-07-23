#ifndef SERVER_UTILS_HEADER
#define SERVER_UTILS_HEADER
#include "rapidjson/document.h"
#include <fstream> 
#include <iostream> 
#include <vector>

using namespace std;
using namespace rapidjson;

vector<string> get_ip_pool() {
	ifstream config_file("aslb_config.json");
	string json((istreambuf_iterator<char>(config_file)), istreambuf_iterator<char>());
	Document doc;
	doc.Parse(json.c_str());

	if (doc.HasParseError()) {
		cerr << "Error parsing `aslb_config.json`: " << doc.GetParseError() << endl;
		throw;
	}

	std::vector<string> ipPool;
	for (const auto& ip : doc["ipPool"].GetArray()) {
		if (ip.IsString()) {
			ipPool.push_back(ip.GetString());
		}
	}
	cout << endl << "--------------------- CONFIG FILE WAS READ -----------------------" << endl;
	return ipPool;
}
#endif // !HEADER_UTILS