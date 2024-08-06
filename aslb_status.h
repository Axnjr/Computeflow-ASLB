#pragma once
#include <mutex>

enum LB_STATUS_TYPE {
    OK,
    SCALING_UP,
    SCALING_DOWN,
    ERR
};

static LB_STATUS_TYPE lb_status = OK;
static std::mutex lb_status_mutex;

void setLBStatus(LB_STATUS_TYPE status) {
    std::lock_guard<std::mutex> lock(lb_status_mutex);
    lb_status = status;
}

std::string getLBStatus() {
    std::lock_guard<std::mutex> lock(lb_status_mutex);
    //return lb_status;
    switch (lb_status) {
        case OK: return "OK";
        case SCALING_UP: return "SCALING_UP";
        case SCALING_DOWN: return "SCALING_DOWN";
        case ERR: return "ERR";
        default: return "UNKNOWN";
    }
}