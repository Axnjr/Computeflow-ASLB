#include <list>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

class ip_cache {

    // constant time insertion and constant time erase, can work as stack / queue.
    // Methods: pop_back, pop_front, push_back, push_back

    public:
        // lru_map has a pair that stores the iterator in the lru and the corresponding index in the ip_pool
        static unordered_map<string, pair<list<string>::iterator, int>> lru_map;
        static list<string> lru;
        static int cap;

    void show_cache()
    {
        cout << endl
            << "THE CACHE: [ ";
        for (auto ip : lru)
        {
            cout << ip << " , ";
        }
        cout << " ]" << endl;
    }

    void add(string ip, int ip_pool_index)
    {
        if (lru.size() == cap)
        {
            lru_map.erase(lru.front());
            lru.pop_front();
        }
        lru.push_front(ip);
        lru_map[ip] = make_pair(lru.begin(), ip_pool_index);
    }

    void update_cache(string ip)
    {
        auto it1 = lru_map[ip].first;
        auto it2 = std::next(it1, 1);
        if (it2 != lru.end())
        {
            // store next val in a temporary var
            auto temp = *it2;
            // swap lru elements
            *it2 = ip;
            *it1 = temp;
            // swap map iterator
            lru_map[ip].first = it2;
            lru_map[temp].first = it1;
        }
    }

    int find_ip(string ip, int& ip_pool_index)
    {
        if (lru_map.find(ip) == lru_map.end())
        {
            add(ip, ip_pool_index);
            return -1;
        }
        else
        { 
            // ip is repeated
            update_cache(ip);
            return lru_map[ip].second;
        }
    }

    void show_cache_analysis(vector<string> ip_pool)
    {
        cout << endl
            << "Client and Server IP's mapping: " << endl;
        for (auto& ci : lru_map)
        {
            cout << "Client Ip: " << ci.first << " Maps to Server Ip: " << ip_pool[ci.second.second] << " * = " << *ci.second.first << endl;
        }
    }
};