#ifndef SERVER_LOAD_BALANCER_HEADER
#define SERVER_LOAD_BALANCER_HEADER

#include "utils.h"
#include <vector>

static vector<string> ip_pool = get_ip_pool();

string get_server_ip(static int& REQUEST_COUNT) {
	if (REQUEST_COUNT >= ip_pool.size()) {
		REQUEST_COUNT = 0;
		cout << "REQUEST COUNT RESET !!" << endl;
	}
	cout << "REQUEST COUNT: " << REQUEST_COUNT << endl;
	return ip_pool[REQUEST_COUNT++];
}

#endif // !SERVER_LOAD_BALANCER_HEADER