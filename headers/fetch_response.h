#ifndef FETCH_RESPONSE_FROM_ORIGINAL_SERVER_H
#define FETCH_RESPONSE_FROM_ORIGINAL_SERVER_H

#include "httplib.h"

using namespace std;
using namespace httplib;

auto fetch_response_from_ip(string ip, string req, int port){
	httplib::Client PROXY_CLIENT(ip); // port to be added
	auto res_forwarded = PROXY_CLIENT.Get(req);
	cout << "IP:" << ip << res_forwarded.error() << endl;
	if (res_forwarded) {
		cout << "STATUS:" << res_forwarded->status << endl;
		cout << "HEADER:" << res_forwarded->get_header_value("Content-Type") << endl;
		cout << "BODY:" << res_forwarded->body << endl;
	}
	return res_forwarded;
}

#endif // !FETCH_RESPONSE_FROM_ORIGINAL_SERVER_H