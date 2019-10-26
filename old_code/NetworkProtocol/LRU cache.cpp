#include <iostream>
#include <list>
#include <unordered_map>
using namespace std;

class LRUcache 
{
public:
    LRUcache(int capacity): cap(capacity) {}

    int get(int key) {
        auto it = m.find(key);
        if(it == m.end()) return -1;
        l.splice(l.begin(), l, it->second);
        return it->second->second;
    }

    void put(int key, int value) {
        auto it = m.find(key);
        if(it != m.end()) l.erase(it->second);
        l.push_front(make_pair(key, value));
        m[key] = l.begin();
        if(m.size() > cap) {
            int k = l.rbegin()->first;
            l.pop_back();
            m.erase(k);
        }
    }

private:
    int cap;
    list<pair<int, int>> l;
    unordered_map<int, list<pair<int, int>>::iterator> m;
};

typedef unordered_map<int, list<pair<int, int>>::iterator> hashMap;

list<pair<int, int>> cache;
hashMap mapp;
int capicity;

int get(int k)
{
    auto it = mapp.find(k);
    if(it == mapp.end()) return -1;
    int res = it->second->second;
    cache.erase(it->second);
    cache.push_front(make_pair(k, res));
    mapp[k] = cache.begin();
    return res;
}

void put(int k, int v)
{
    auto it = mapp.find(k);
    if(it != mapp.end()) { it->second->second = v; }//insert doesn't count in manipulate;
    else {
        if(cache.size() == capicity) {
            int key = cache.back().first;
            cache.pop_back();
            mapp.erase(key);
        }
        cache.push_front(make_pair(k, v));
        mapp[k] = cache.begin();
    }
}

int main()
{
    int k, v;
    char c;
    cin >> capicity;
    while(cin >> c) {
        if(c == 'p') {
            cin >> k >> v;
            if(capicity <= 0) continue;
            put(k, v);
        }
        if(c == 'g') {
            cin >> k;
            cout << get(k) << endl;
        }
    }
    return 0;
}

