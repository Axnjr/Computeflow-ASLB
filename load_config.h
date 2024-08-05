#ifndef SERVER_CONFIG_LOADER_H
#define SERVER_CONFIG_LOADER_H

#include <vector>
#include <fstream> 
#include <iostream> 
#include "rapidjson/document.h"
#include "lb_config_struct.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "logger.h"

using namespace std;
using namespace rapidjson;

string validate_config_json(const Document& doc) {

    if (doc.HasParseError()) return "JSON PARSE ERROR";

    if (!doc.HasMember("port") || !doc["port"].IsInt()) return "port";
    if (!doc.HasMember("maxVms") || !doc["maxVms"].IsInt()) return "maxVms";
    if (!doc.HasMember("minVms") || !doc["minVms"].IsInt()) return "minVms";
    if (!doc.HasMember("vmCount") || !doc["vmCount"].IsInt()) return "vmCount";
    if (!doc.HasMember("amiId") || !doc["amiId"].IsString()) return "amiId";
    if (!doc.HasMember("ipPoolDev") || !doc["ipPoolDev"].IsArray()) return "ipPollDev";
    if (!doc.HasMember("vmMetaData") || !doc["vmMetaData"].IsObject()) return "vmMetaData";
    if (!doc.HasMember("scalingPolicies") || !doc["scalingPolicies"].IsObject()) return "scalingPolicies";

    const Value& vmMetaData = doc["vmMetaData"];
    if (!vmMetaData.HasMember("enviromentVariables") || !vmMetaData["enviromentVariables"].IsString()) return "enviromentVariables";
    if (!vmMetaData.HasMember("script") || !vmMetaData["script"].IsString()) return "script";
    if (!vmMetaData.HasMember("vmType") || !vmMetaData["vmType"].IsString()) return "vmType";

    const Value& scalingPolicies = doc["scalingPolicies"];
    if (!scalingPolicies.HasMember("maxCPU") || !scalingPolicies["maxCPU"].IsFloat()) return "maxCPU";
    if (!scalingPolicies.HasMember("minCPU") || !scalingPolicies["minCPU"].IsFloat()) return "minCPU";
    if (!scalingPolicies.HasMember("maxMemory") || !scalingPolicies["maxMemory"].IsFloat()) return "maxMemory";
    if (!scalingPolicies.HasMember("minMemory") || !scalingPolicies["minMemory"].IsFloat()) return "minMemory";

    return "";
}

void initialize_static_memory_from_config() {

    using namespace logger;

    ifstream config_file("aslb_config.json");

    if (!config_file.is_open()) {
        ltf("\n Unable to open `aslb_config.json` \n");
        throw runtime_error("Unable to open config file");
    }

    string json((istreambuf_iterator<char>(config_file)), istreambuf_iterator<char>());

    Document doc;
    doc.Parse(json.c_str());

    string validation_response = validate_config_json(doc);
    if (!validation_response.empty()) {
        ltf(
            "\n Error Validating `aslb.config.json` file. Either the property: `",
            validation_response,
            "` is missing or has wrong data type !! \n"
        );
    }

    ltf("\n DATA LOADED FROM CONFIG IN `dev` MODE: REPLACE `ipPoolDev` WITH `ipPool` \n");

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

    const Value& vmMetaData = doc["vmMetaData"];

    LB_CONFIG::env = vmMetaData["enviromentVariables"].GetString();
    LB_CONFIG::script = vmMetaData["script"].GetString();
    LB_CONFIG::vm_type = vmMetaData["vmType"].GetString();

    const Value& scalingPolicies = doc["scalingPolicies"];

    LB_CONFIG::max_cpu_usage = scalingPolicies["maxCPU"].GetFloat();
    LB_CONFIG::max_mem_usage = scalingPolicies["maxMemory"].GetFloat();
    LB_CONFIG::min_cpu_usage = scalingPolicies["minCPU"].GetFloat();
    LB_CONFIG::min_mem_usage = scalingPolicies["minMemory"].GetFloat();

    ltf("\n -------------------------- CONFIG FILE READ SUCCESSFULLY --------------------------- \n");

    ltf("\n PORT:          "                  ,LB_CONFIG::PORT                                 ,"\n");
    ltf("\n ENV:           "                  ,LB_CONFIG::env                                  ,"\n");
    ltf("\n SCRIPT:        "                  ,LB_CONFIG::script                               ,"\n");
    ltf("\n VM_TYPE:       "                  ,LB_CONFIG::vm_type                              ,"\n");
    ltf("\n MAX_CPU_USAGE: "                  ,LB_CONFIG::max_cpu_usage                        ,"\n");
    ltf("\n MIN_CPU_USAGE: "                  ,LB_CONFIG::min_cpu_usage                        ,"\n");
    ltf("\n MAX_MEM_USAGE: "                  ,LB_CONFIG::max_mem_usage                        ,"\n");
    ltf("\n MIN_MEM_USAGE: "                  ,LB_CONFIG::min_mem_usage                        ,"\n");

    for (const auto& ip : LB_CONFIG::IP_POOL) {
    ltf("\n IP:            "                  ,ip                                              ,"\n");
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