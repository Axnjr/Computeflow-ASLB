#ifndef SERVER_CONFIG_LOADER_H
#define SERVER_CONFIG_LOADER_H

#include <vector>
#include <fstream> 
#include <iostream> 
#include "rapidjson/document.h"
#include "lb_config_struct.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

void initialize_static_memory_from_config() {

    ifstream config_file("aslb_config.json");

    if (!config_file.is_open()) {
        cerr << "Unable to open `aslb_config.json`" << endl;
        throw runtime_error("Unable to open config file");
    }

    string json((istreambuf_iterator<char>(config_file)), istreambuf_iterator<char>());

    Document doc;
    doc.Parse(json.c_str());

    if (doc.HasParseError()) {
        cerr << "Error parsing `aslb_config.json`: " << doc.GetParseError() << endl;
        throw runtime_error("JSON parse error");
    }

    cout << endl << "DATA LOADED FROM CONFIG IN `dev` MODE: REPLACE `ipPoolDev` WITH `ipPool`" << endl;

    if (!doc.HasMember("ipPoolDev") || !doc["ipPoolDev"].IsArray()) {
        cerr << "Invalid JSON: 'ipPoolDev' property is missing or not an array" << endl;
        throw runtime_error("Invalid JSON: 'ipPoolDev' property");
    }

    for (const auto& ip : doc["ipPoolDev"].GetArray()) {
        if (ip.IsString()) {
            LB_CONFIG::IP_POOL.push_back(ip.GetString());
        }
    }

    LB_CONFIG::PORT = doc["port"].GetInt();
    LB_CONFIG::maxVms = doc["maxVms"].GetInt();
    LB_CONFIG::minVms = doc["minVms"].GetInt();
    LB_CONFIG::vmCount = doc["vmCount"].GetInt();
    LB_CONFIG::ami_id = doc["amiId"].GetString();

    if (doc.HasMember("vmMetaData") && doc["vmMetaData"].IsObject()) {

        const Value& vmMetaData = doc["vmMetaData"];

        if (vmMetaData.HasMember("enviromentVariables")) {
            LB_CONFIG::env = vmMetaData["enviromentVariables"].GetString();
        }

        if (vmMetaData.HasMember("script")) {
            LB_CONFIG::script = vmMetaData["script"].GetString();
        }

        if (vmMetaData.HasMember("vmType")) {
            LB_CONFIG::vm_type = vmMetaData["vmType"].GetString();
        }

        /*if (vmMetaData.HasMember("vmIds")) {
            for (const auto& id : doc["vmIds"].GetArray()) {
                if (id.IsString()) {
                    LB_CONFIG::IP_POOL.push_back(id.GetString());
                }
            }
        }*/
    }

    if (doc.HasMember("scalingPolicies")) {

        const Value& scalingPolicies = doc["scalingPolicies"];

        if (scalingPolicies.HasMember("maxCPU")) {
            LB_CONFIG::max_cpu_usage = scalingPolicies["maxCPU"].GetFloat();
        }

        if (scalingPolicies.HasMember("maxMemory")) {
            LB_CONFIG::max_mem_usage = scalingPolicies["maxMemory"].GetFloat();
        }

        if (scalingPolicies.HasMember("minCPU")) {
            LB_CONFIG::min_cpu_usage = scalingPolicies["minCPU"].GetFloat();
        }

        if (scalingPolicies.HasMember("minMemory")) {
            LB_CONFIG::min_mem_usage = scalingPolicies["minMemory"].GetFloat();
        }
    }

    cout << endl << "--------------------- CONFIG FILE READ SUCCESSFULLY -----------------------" << endl;

    cout << "PORT: " << LB_CONFIG::PORT << endl;
    cout << "ENV: " << LB_CONFIG::env << endl;
    cout << "SCRIPT: " << LB_CONFIG::script << endl;
    //cout << "VM_ID: " << LB_CONFIG::vm_id << endl;
    cout << "VM_TYPE: " << LB_CONFIG::vm_type << endl;
    cout << "MAX_CPU_USAGE: " << LB_CONFIG::max_cpu_usage << endl;
    cout << "MIN_CPU_USAGE: " << LB_CONFIG::min_cpu_usage << endl;
    cout << "MAX_MEM_USAGE: " << LB_CONFIG::max_mem_usage << endl;
    cout << "MIN_MEM_USAGE: " << LB_CONFIG::min_mem_usage << endl;

    for (const auto& ip : LB_CONFIG::IP_POOL) {
        cout << "IP: " << ip << endl;
    }
}

string getLBConfigAsJson() {

    Document doc;
    doc.SetObject();
    Document::AllocatorType& allocator = doc.GetAllocator();

    doc.AddMember("PORT", LB_CONFIG::PORT, allocator);

    Value ipPool(kArrayType);
    for (const auto& ip : LB_CONFIG::IP_POOL) {
        ipPool.PushBack(Value().SetString(ip.c_str(), allocator), allocator);
    }
    doc.AddMember("IP_POOL", ipPool, allocator);

    doc.AddMember("env", Value().SetString(LB_CONFIG::env.c_str(), allocator), allocator);
    doc.AddMember("script", Value().SetString(LB_CONFIG::script.c_str(), allocator), allocator);
    //doc.AddMember("vm_id", Value().SetString(LB_CONFIG::vm_id.c_str(), allocator), allocator);
    doc.AddMember("vm_type", Value().SetString(LB_CONFIG::vm_type.c_str(), allocator), allocator);

    doc.AddMember("max_cpu_usage", LB_CONFIG::max_cpu_usage, allocator);
    doc.AddMember("min_cpu_usage", LB_CONFIG::min_cpu_usage, allocator);
    doc.AddMember("max_mem_usage", LB_CONFIG::max_mem_usage, allocator);
    doc.AddMember("min_mem_usage", LB_CONFIG::min_mem_usage, allocator);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);

    return buffer.GetString();
}

#endif // !SERVER_CONFIG_LOADER_H