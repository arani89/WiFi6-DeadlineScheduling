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

// class InputParser{
//     public:
//         InputParser (int &argc, char **argv){
//             for (int i=1; i < argc; ++i)
//                 this->tokens.push_back(std::string(argv[i]));
//         }
//         /// @author iain
//         const std::string& getCmdOption(const std::string &option) const{
//             std::vector<std::string>::const_iterator itr;
//             itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
//             if (itr != this->tokens.end() && ++itr != this->tokens.end()){
//                 return *itr;
//             }
//             static const std::string empty_string("");
//             return empty_string;
//         }
//         /// @author iain
//         bool cmdOptionExists(const std::string &option) const{
//             return std::find(this->tokens.begin(), this->tokens.end(), option)
//                    != this->tokens.end();
//         }
//     private:
//         std::vector <std::string> tokens;
// };

void fitPacketsToEmptySpaces(vector<IntervalData> &itvl_data, vector<Packet> &packets1, vector<Packet> &packets2, int start_time, int end_time)
{
    int p1len = packets1.size();
    int p2len = packets2.size();
    int itvlen = itvl_data.size();
    int idx1 = 0;
    int idx2 = 0;
    // schedule packets1 first
    for(int i = -1; i < itvlen; i++){
        double start = (i == -1)? start_time: itvl_data[i].end;
        double end = (i+1 == itvlen)? end_time : itvl_data[i+1].start;
        while(start < end){
            if(idx1 == p1len && idx2 == p2len) break;  // end of ND Packets

            // Schedule packets1 before packets 2 (FCFS)
            Packet &pckt = (idx1 < p1len)? packets1[idx1] : packets2[idx2];
            double reqtime = calcTransmissionTimeMs(pckt.datasize, RU_1992, pckt.stationId);
            if(start+reqtime <= end){
                //schedule it
                schedulePacket(pckt, RU_1992, reqtime);
                pckt.setNDP();   // could have done this someplace better.
                // updateNDPDetails(pckt, start, start+reqtime);

                start += reqtime;
                if(idx1<p1len) idx1++;
                else idx2++;
            } else {
                //fragment and schedule it
                int dataTransferrable = dataTransferrableBytes(RU_1992, pckt.stationId, (end-start)/granularity);
                if(dataTransferrable >= pckt.datasize){
                    cout<<"Error, packet can be scheduled but hasn't been."; exit(EXIT_FAILURE);
                }
                if(dataTransferrable > 0){
                    schedulePacket(pckt, RU_1992, end-start);
                    // pckt.setNDP();
                    int tempdatasize = pckt.datasize - dataTransferrable; 
                    pckt.datasize = dataTransferrable;
                    // updateNDPDetails(pckt, start, end);
                    pckt.datasize = tempdatasize;
                }
                break;
            }
        }
    }

    // idx1 packets scheduled for packet1, and idx2 for packets2
    vector<Packet> newpckts1, newpckts2;
    for(int i = idx1; i < p1len; i++){
        newpckts1.push_back(packets1[i]);
    }
    for(int i = idx2; i < p2len; i++){
        newpckts2.push_back(packets2[i]);
    }
    swap(packets1, newpckts1);
    swap(packets2, newpckts2);
}
void fitPacketsToFreeRUs(vector<IntervalData> &itvl_data, vector<Packet> &packets)
{
    int plen = packets.size();
    int currentp = 0;
    for(IntervalData &itvl : itvl_data)
    {
        if(currentp == plen) break; // reached the end
        Packet &pckt = packets[currentp];
        // try to assign this packet
        bool assigned = false;
        for (int mode = RU_26; mode >= RU_1992; --mode)
        {
            if (itvl.freeslots[mode] <= 0)
                continue;
            if (packetMatchesInterval(itvl.start, itvl.end, pckt, mode, granularity))
            {
                itvl.freeslots[mode]--;
                assigned = true;
                itvl.insert(pckt);
                schedulePacket(pckt, mode, itvl.end-itvl.start);
                pckt.setNDP();
                // updateNDPDetails(pckt, itvl.start, itvl.end);
                break;
            }
        }
        if (assigned) {
            currentp++;
        } 
    }
    vector<Packet> newpckts;
    for(int i = currentp; i < plen; i++){
        newpckts.push_back(packets[i]);
    }
    swap(packets, newpckts);
}

int main(int argc, char **argv)
{
    // InputParser input(argc, argv);
    // Config userconfig;
    // if(input.cmdOptionExists("-b")) {
    //     string bandwidth = input.getCmdOption("-b");
    //     const int bandwidth_val = stoi(bandwidth); 
    //     if (bandwidth_val == 160) {
    //         int config_array[] = {1, 0, 0, 0, 0, 0, 0};
    //         userconfig.config.assign(config_array, config_array + 7); 
    //         userconfig.maxRU = RU_1992;
    //         int masterconfig_array[] = {1, 2, 4, 8, 16, 32, 74};
    //         userconfig.masterconfig.assign(masterconfig_array, masterconfig_array + 7); 
    //     }
    //     else {
    //         int config_array[] = {1, 0, 0, 0, 0};
    //         userconfig.config.assign(config_array, config_array + 5); 
    //         userconfig.maxRU = RU_484;
    //         int masterconfig_array[] ={1, 2, 4, 8, 18};
    //         userconfig.masterconfig.assign(masterconfig_array, masterconfig_array + 5);
    //     }
    // }
    // else {
    //     int config_array[] = {1, 0, 0, 0, 0};
    //     userconfig.config.assign(config_array, config_array + 5); 
    //     userconfig.maxRU = RU_484;
    //     int masterconfig_array[] ={1, 2, 4, 8, 18};
    //     userconfig.masterconfig.assign(masterconfig_array, masterconfig_array + 5);
    // }
	
    	
    int T; cin>>T;
    int inputcase;
    cin >> inputcase;
    vector<int> config = {1, 0, 0, 0, 0, 0, 0};
    createRUConfigs(0, config
    // , userconfig
    );
    int timeperiod;
    cin >> timeperiod;
    cin >> granularity;
    cin >> criticalThreshold;

    vector<vector<double>> producers; // to store input details for multiple runs of createBatch
    vector<Packet> packets;
    // currentPackets.reserve(600000);
    /*
        we are assuming non Determinstic packets are all of same profit
        therefore counting the number of non Deterministic packets scheduled
        should be enough.

        nonDeterministicPackets stores the ndp coming per round
   */
    stations_count = createBatch(packets, 0, timeperiod, inputcase, producers, criticalThreshold);
    int maxScorePossible = 0;
    for(Packet& pckt: packets){
        maxScorePossible += pckt.penalty;
    }
    cout<<"Max profit : "<<maxScorePossible<<"\n";
    cout<<"Total packets : "<< total_packets<<"\n";
    // DPMSS
    auto start = high_resolution_clock::now();
    DPMSS(packets, 0, timeperiod, stations_count, criticalThreshold, granularity, -1
    // , userconfig
    );
    auto t1 = high_resolution_clock::now();
    // EDF
    newBaseline(packets, 0, timeperiod, EDF, stations_count, criticalThreshold);
    // LRF
    newBaseline(packets, 0, timeperiod, LRF, stations_count, criticalThreshold);
    // NLRF
    baselineNLRF(packets, 0, timeperiod, stations_count, criticalThreshold);
    configs = {{0, 0, 0, 0, 0, 0, 74}};
    auto t2 = high_resolution_clock::now();
    DPMSS(packets, 0, timeperiod, stations_count, criticalThreshold, granularity, 0.25);
    auto t3 = high_resolution_clock::now();
    auto dur1 = duration_cast<microseconds>(t1 - start);
    auto dur2 = duration_cast<microseconds>(t3 - t2);
    cout<<"LSDS algo time taken: \n"<<(dur1.count())/1000<<"\n"; //milliseconds
    cout<<"LSDSF algo time taken: \n"<<(dur2.count())/1000<<"\n";
    // cout<<"---------\n";
}
