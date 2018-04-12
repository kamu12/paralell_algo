#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <thread>
#include <future>
#include <chrono>
#include <random>
#include <algorithm>
#include <iterator>
#include <functional>
using namespace std;

using MapRes = map<int, vector<int> >;
using VectorRes = vector<pair<int, vector<int> > >;

MapRes mapper(vector<int>& arr, int start, int end)
{
    MapRes res;
    for(int i = start; i < end; i++)
    {
        res[arr[i]].push_back(1);
    }

    return res;
}

map<int, int> reducer(VectorRes& mapped_list)
{
    map<int, int> res;
    int sum_el;
    for(auto el : mapped_list)
    {
        sum_el = 0;
        for (auto& n : el.second)
        {
            sum_el += n;
            res[el.first] = sum_el;
        }
    }
    return res;
    // for(auto x: res)
    // {
    //     cout << x.first << "=>" << x.second << endl;
    // }
}

void print_mapper_res(auto mapped_list)
{
    for(auto x : mapped_list)
        {
            cout << x.first << "=>";
            for(auto& el: x.second)
            {
                cout << el << " ";
            }
            cout << endl;
        }
        cout << "-------------------------" << endl;
}

template<typename R>
  bool is_ready(std::future<R> const& f)
  { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }

int main()
{
    random_device rnd_device;
    mt19937 mersenne_engine(rnd_device());
    uniform_int_distribution<int> dist(0, 9);

    auto gen = std::bind(dist, mersenne_engine);
    vector<int> arr(1024);
    generate(begin(arr), end(arr), gen);

    size_t n = thread::hardware_concurrency();
    
    //map part
    future<MapRes> t[n];
    unsigned part = arr.size() / n;

    for(int i = 0; i < n; ++ i)
    {
        int start = i * part;
        int end = start + part;
        t[i] = async(mapper, ref(arr), start, end);
    }
    // shuffle & sort
    MapRes merged;
    for(int i = 0; i < n; ++ i)
    {
        auto mapped_list = t[i].get();
        print_mapper_res(mapped_list);
        for(auto x: mapped_list)
        {
            merged[x.first].insert(merged[x.first].end(), x.second.begin(), x.second.end());
        }
        
    }
    print_mapper_res(merged);

    // reduce
    future<map<int, int>> pool[n];
    VectorRes vect;
    copy(merged.begin(), merged.end(), back_inserter(vect));
    auto begin = vect.begin();
    auto end = vect.end();
    int map_part = 3;
    for(int i = 0; i < 3; i++)
    {
        VectorRes partial(begin, begin + map_part);
        begin += map_part;
        pool[i] = async(reducer, ref(partial));
        pool[i].wait();
    }

    // reduce what remains
    VectorRes partial(begin, end);
    pool[3] = async(reducer, ref(partial));

    map<int, int> fin;
    for(int i = 0; i < n; ++ i)
    {
        pool[i].wait();
        if(is_ready(pool[i]))
        {
            auto reducer_part = pool[i].get();
            for(auto x: reducer_part)
            {
                cout << x.first << "=>" << x.second << endl;
            }
        }
    }
}