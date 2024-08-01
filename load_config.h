#ifndef SERVER_CONFIG_LOADER_H
#define SERVER_CONFIG_LOADER_H

#include "rapidjson/document.h"
#include <vector>
#include <fstream> 
#include <iostream> 

using namespace std;
using namespace rapidjson;

void initialize_static_memory_from_config(int& PORT, vector<string>& IP_POOL) {
	ifstream config_file("aslb_config.json");
	string json((istreambuf_iterator<char>(config_file)), istreambuf_iterator<char>());

	Document doc;
	doc.Parse(json.c_str());

	if (doc.HasParseError()) {
		cerr << "Error parsing `aslb_config.json`: " << doc.GetParseError() << endl;
		throw doc.GetParseError();
	}

	cout << endl << "⚠️👀✌️💸 DATA LOADED FROM CONFIG IN `dev` MODE: REPLACE `ipPoolDev` WITH `ipPool` 💵🤞😓" << endl;

	for (const auto& ip : doc["ipPoolDev"].GetArray()) { // change to `ipPool` in production ⚠️👀
		if (ip.IsString()) {
			IP_POOL.push_back(ip.GetString());
		}
	}

	PORT = doc["port"].GetInt();

	cout << endl << "--------------------- CONFIG FILE WAS READ -----------------------" << endl;
	cout << "PORT: " << PORT << endl;
}

#endif // !SERVER_CONFIG_LOADER_H