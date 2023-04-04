#include <iostream>
#include <set>
#include <vector>
#include <queue>
#include <cmath>
#include <stack>
#include <algorithm>
#include <chrono>
#include <random>
#include <string>

#include "./header_files/IntervalData.hpp"
#include "./header_files/Packet.hpp"
#include "./header_files/taking_input.hpp"
#include "./header_files/AllConfigs.hpp"
#include "./header_files/Baselines.hpp"
#include "./header_files/DPMSS.hpp"
#include "./header_files/commonDefines.hpp"

using namespace std;
using namespace std::chrono;

struct Interval
{
    int start;
    int end;
};

//------------------- Interval Generation -------------------

// global variables:
double granularity = 1; // (1/granularity)ms gives the smallest unit of time considered in this algorithm
int stations_count = 0;
int criticalThreshold;
bool printdetails = 0;
int n_rounds = 4; // number of rounds


int main(int argc, char **argv)
{
    InputParser input(argc, argv);
    Config userconfig;
    if(input.cmdOptionExists("-b")) {
        string bandwidth = input.getCmdOption("-b");
        const int bandwidth_val = stoi(bandwidth); 
        if (bandwidth_val == 160) {
            int config_array[] = {0, 0, 0, 0, 0, 0, 1};
            userconfig.config.assign(config_array, config_array + 7); 
            userconfig.maxRU = RU_1992;
            int masterconfig_array[] = {74, 32, 16, 8, 4, 2, 1};
            userconfig.masterconfig.assign(masterconfig_array, masterconfig_array + 7); 
        }
        else {
            int config_array[] = {0, 0, 0, 0, 1};
            userconfig.config.assign(config_array, config_array + 5); 
            userconfig.maxRU = RU_484;
            int masterconfig_array[] ={18, 8, 4, 2, 1};
            userconfig.masterconfig.assign(masterconfig_array, masterconfig_array + 5);
        }
    }
    else {
        int config_array[] = {0, 0, 0, 0, 1};
        userconfig.config.assign(config_array, config_array + 5); 
        userconfig.maxRU = RU_484;
        int masterconfig_array[] ={18, 8, 4, 2, 1};
        userconfig.masterconfig.assign(masterconfig_array, masterconfig_array + 5);
    }
    int T; cin>>T;
    int inputcase;
    cin >> inputcase;
    createRUConfigs(userconfig.maxRU, userconfig);
    int timeperiod;
    cin >> timeperiod;
    cin >> granularity;
    cin >> criticalThreshold;

    vector<vector<double>> producers; // to store input details for multiple runs of createBatch
    vector<Packet> packets;

    stations_count = createBatch(packets, 0, timeperiod, inputcase, producers, criticalThreshold);
    int maxScorePossible = 0;
    for(Packet& pckt: packets){
        maxScorePossible += pckt.penalty;
    }
    cout<<"Max profit : "<<maxScorePossible<<"\n";
    cout<<"Total packets : "<< total_packets<<"\n";
    // DPMSS
    auto start = high_resolution_clock::now();
    DPMSS(packets, 0, timeperiod, stations_count, criticalThreshold, granularity, -1, userconfig);
    auto t1 = high_resolution_clock::now();
    // EDF
    newBaseline(packets, 0, timeperiod, EDF, stations_count, criticalThreshold, userconfig);
    // LRF
    newBaseline(packets, 0, timeperiod, LRF, stations_count, criticalThreshold, userconfig);
    // NLRF
    baselineNLRF(packets, 0, timeperiod, stations_count, criticalThreshold, userconfig);
    configs = {{0, 0, 0, 0, 0, 0, 74}};
    auto t2 = high_resolution_clock::now();
    DPMSS(packets, 0, timeperiod, stations_count, criticalThreshold, granularity, -1, userconfig);
    auto t3 = high_resolution_clock::now();
    auto dur1 = duration_cast<microseconds>(t1 - start);
    auto dur2 = duration_cast<microseconds>(t3 - t2);
    cout<<"LSDS algo time taken: \n"<<(dur1.count())/1000<<"\n"; //milliseconds
    cout<<"LSDSF algo time taken: \n"<<(dur2.count())/1000<<"\n";
    // cout<<"---------\n";
}
