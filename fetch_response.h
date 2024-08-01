#ifndef FETCH_RESPONSE_FROM_ORIGINAL_SERVER_H
#define FETCH_RESPONSE_FROM_ORIGINAL_SERVER_H

#include "dependencies/httplib.h"

using namespace std;
using namespace httplib;

auto fetch_response_from_ip(string ip, string req, int port){
	cout << endl << "REQ-PATH:" << req << endl;
	httplib::Client PROXY_CLIENT("localhost", 3000); // port to be added
	auto res_forwarded = PROXY_CLIENT.Get(req);
	cout << "IP:" << ip << res_forwarded.error() << endl;
	return res_forwarded;
}

#endif // !FETCH_RESPONSE_FROM_ORIGINAL_SERVER_H