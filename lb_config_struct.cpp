#include "lb_config_struct.h"

int LB_CONFIG::PORT = 0;
int LB_CONFIG::maxVms = 100;
int LB_CONFIG::minVms = 1;
int LB_CONFIG::vmCount = 0;

vector<string> LB_CONFIG::IP_POOL = {};
//vector<string> LB_CONFIG::vmIds = {};

string LB_CONFIG::env = "";
string LB_CONFIG::script = "";
string LB_CONFIG::vm_type = "";
string LB_CONFIG::ami_id = "";
string LB_CONFIG::mail_id = "";

float LB_CONFIG::max_cpu_usage = 0.0f;
float LB_CONFIG::min_cpu_usage = 0.0f;
float LB_CONFIG::max_mem_usage = 0.0f;
float LB_CONFIG::min_mem_usage = 0.0f;