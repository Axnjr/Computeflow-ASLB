#ifndef SERVER_IP_CACHE_H
#define SERVER_IP_CACHE_H
#include <vector>
#include <iostream>
#include <list>
#include <unordered_map>

using namespace std;

class ip_cache {

    public:
    // constant time insertion and constant time erase, can work as stack / queue. 
    // Methods: pop_back, pop_front, push_back, push_back
    static list<string> lru; 
    static unordered_map<string, pair<list<string>::iterator, int>> lru_map;
    static int cap;


    void add(string ip, int ip_pool_index) {

        auto ip_ref = lru_map.find(ip);

        if (ip_ref != lru_map.end()) {
            lru.erase(ip_ref->second.first);
            lru.push_back(ip);                                  // more fetched ip's stay at the back :)
            lru_map[ip] = make_pair(lru.end(), ip_pool_index);  // update ip location in the map
        }

        else if (lru.size() == cap) {
            lru_map.erase(lru.front());
            lru.pop_front();
            lru.push_front(ip);
        }

        lru.push_front(ip);
        lru_map.insert(ip, lru.front());
    }

    int find_ip(string ip, int& ip_pool_index) {

        auto ip_ref = lru_map.find(ip);

        if (ip_ref == lru_map.end()) {
            add(ip, ip_pool_index);
            return -1;
        }
        return ip_ref->second.second;
    }

    void show_cache(vector<string> ip_pool) {
        cout << endl << "IP CACHE: " << endl;
        for (auto& ci : lru_map) {
            cout << "Client Ip: " << ci.first << " Maps to Server Ip: " << ip_pool[ci.second.second] << endl;
        }
    }
};

#endif // SERVER_IP_CACHE_H