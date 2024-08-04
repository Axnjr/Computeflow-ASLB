#pragma once
#ifndef LB_CONFIG_H
#define LB_CONFIG_H

#include <vector>
#include <string>
#include <chrono>

using namespace std;

struct LB_CONFIG {

    static int PORT;
    static int maxVms;
    static int minVms;
    static int vmCount;

    static vector<string> IP_POOL;
    //static vector<string> vmIds;

    static string env;
    static string script;
    static string vm_type;
    static string ami_id;

    static float max_cpu_usage;
    static float min_cpu_usage;
    static float max_mem_usage;
    static float min_mem_usage;
};

#endif // LB_CONFIG_H