#pragma once
#include <iostream>
#include <string>
#include <cstdlib>
#include <curl/curl.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "logger.h"
#include "lb_config_struct.h"

using namespace logger;

string loadEnv(string which)
{
    std::ifstream file(".env");

    if (!file.is_open()) {
        std::cerr << "Failed to open .env file." << std::endl;
        return "error";
    }

    std::string value1, value2;

    std::getline(file, value1);
    std::getline(file, value2);
    file.close();
    return which == "RESEND" ? value1 : value2;
}

// TO BE IMPLEMENTED
bool add_notification_to_db(string event) 
{
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) 
    {
        struct curl_slist* headers = NULL;
        std::string apiKeyHeader = "Authorization: Bearer " + loadEnv("RESEND");
        std::string contentTypeHeader = "Content-Type: application/json";

        headers = curl_slist_append(headers, apiKeyHeader.c_str());
        headers = curl_slist_append(headers, contentTypeHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, "https://yakshit-chhipa-s-workspace-p8bvk5.eu-central-1.xata.sh/db/cheapcloud:main/tables/notifications/query");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, std::string("{\"event\":\"" + event + "\"}"));

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            ltf("Error logging event to database: " + std::string(curl_easy_strerror(res)));
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    //ltf("Event logged to database successfully!");
    return true;
}


bool notify_via_email(const std::string subject, const std::string body)
{
    std::string from_email = "Computeflow-LoadBalancer <onboarding@resend.dev>";
    std::string to_email = LB_CONFIG::mail_id;

    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        struct curl_slist* headers = NULL;
        std::string apiKeyHeader = "Authorization: Bearer " + loadEnv("RESEND");
        std::string contentTypeHeader = "Content-Type: application/json";

        headers = curl_slist_append(headers, apiKeyHeader.c_str());
        headers = curl_slist_append(headers, contentTypeHeader.c_str());

        rapidjson::Document email_data;
        email_data.SetObject();
        rapidjson::Document::AllocatorType& allocator = email_data.GetAllocator();

        email_data.AddMember("from", rapidjson::Value(from_email.c_str(), allocator), allocator);
        email_data.AddMember("to", rapidjson::Value(to_email.c_str(), allocator), allocator);
        email_data.AddMember("subject", rapidjson::Value(subject.c_str(), allocator), allocator);
        email_data.AddMember("text", rapidjson::Value(body.c_str(), allocator), allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        email_data.Accept(writer);
        std::string email_data_str = buffer.GetString();

        std::cout << std::endl << email_data_str << std::endl;

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.resend.com/emails");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, email_data_str.c_str());

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            ltf("UNBALE TO NOTIFY SCALING UPDATE: ", curl_easy_strerror(res));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    //add_notification_to_db(subject);
    return true;
}