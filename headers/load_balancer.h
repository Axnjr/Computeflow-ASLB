#ifndef SERVER_LOAD_BALANCER_H
#define SERVER_LOAD_BALANCER_H
#include <vector>
#include <iostream>

using namespace std;

// Sticky Session to be implmented
string get_server_ip(int& REQUEST_COUNT, const vector<string>& ip_pool) {
    if (REQUEST_COUNT >= ip_pool.size()) {
        REQUEST_COUNT = 0;
        cout << "REQUEST COUNT RESET !!" << endl;
    }
    cout << "REQUEST COUNT: " << REQUEST_COUNT << endl;
    return ip_pool[REQUEST_COUNT++];
}

#endif // SERVER_LOAD_BALANCER_H