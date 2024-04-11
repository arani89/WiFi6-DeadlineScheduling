#include <iostream>
#include <set>
#include <vector>
#include <queue>
#include <cmath>
#include <stack>
#include <algorithm>
#include <chrono>
#include <random>

#include "./header_files/IntervalData.hpp"
#include "./header_files/Packet.hpp"
#include "./header_files/taking_input.hpp"
#include "./header_files/AllConfigs.hpp"
#include "./header_files/Baselines.hpp"
#include "./header_files/DPMSS.hpp"

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
int packets_scheduled2 = 0;
int packets_scheduled1 = 0;
int totalnoncritical = 0;
int critical_packets_scheduled = 0;
int noncritical_packets_scheduled = 0;
void fitPacketsToEmptySpaces(vector<IntervalData> &itvl_data, vector<Packet> &packets1, vector<Packet> &packets2, int start_time, int end_time, Config &userconfig)
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
            double reqtime = calcTransmissionTimeMs(pckt.datasize, userconfig.maxRU, pckt.stationId, userconfig.mcs_range, userconfig.mcs_lowerval);
            if(start+reqtime <= end){
                //schedule it
                schedulePacket(pckt, userconfig.maxRU, reqtime);
                packets_scheduled1++;
                pckt.setNDP();   // could have done this someplace better.
                transmitNDPacket(pckt, start, start+reqtime);

                start += reqtime;
                if(idx1<p1len) idx1++;
                else idx2++;
            } else {
                //fragment and schedule it
                double dataTransferrable = dataTransferrableBytes(userconfig.maxRU, pckt.stationId, (end-start)/granularity, userconfig.mcs_range, userconfig.mcs_lowerval);
                if(dataTransferrable >= pckt.datasize){
                    cout<<"Error, packet can be scheduled but hasn't been."; exit(EXIT_FAILURE);
                }
                if((int)dataTransferrable > 0){
                    schedulePacket(pckt, userconfig.maxRU, end-start);
                    packets_scheduled1++;
                    pckt.setNDP();
                    double tempdatasize = pckt.datasize - dataTransferrable; 
                    pckt.datasize = dataTransferrable;
                    transmitNDPacket(pckt, start, end);
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
void fitPacketsToFreeRUs(vector<IntervalData> &itvl_data, vector<Packet> &packets, double packetDropFactor, Config &userconfig)
{
    int plen = packets.size();
    int currentp = 0;
    for(IntervalData &itvl : itvl_data)
    {
        if(currentp == plen) break; // reached the end
        Packet &pckt = packets[currentp];
        // try to assign this packet
        bool assigned = false;
        for (int mode = RU_26; mode <= userconfig.maxRU; ++mode)
        {
            if (itvl.freeslots[mode] <= 0)
                continue;
            if (packetMatchesInterval(itvl.start, itvl.end, pckt, mode, granularity, userconfig))
            {
                itvl.freeslots[mode]--;
                assigned = true;
                itvl.insert(pckt);
                schedulePacket(pckt, mode, itvl.end-itvl.start);
                packets_scheduled1++;
                pckt.setNDP();
                transmitNDPacket(pckt, itvl.start, itvl.end);
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
    InputParser input(argc, argv);
    Config userconfig;
    if(input.cmdOptionExists("-bw")) {
        string bandwidth = input.getCmdOption("-bw");
        const int bandwidth_val = stoi(bandwidth); 
        if (bandwidth_val == 160) {
            int config_array[] = {0, 0, 0, 0, 0, 0, 1};
            userconfig.config.assign(config_array, config_array + 7); 
            userconfig.maxRU = RU_1992;
            int masterconfig_array[] = {74, 32, 16, 8, 4, 2, 1};
            userconfig.masterconfig.assign(masterconfig_array, masterconfig_array + 7); 
        } else if(bandwidth_val == 80){
            int config_array[] = {0, 0, 0, 0, 0, 1};
            userconfig.config.assign(config_array, config_array + 6);
            userconfig.maxRU = RU_996;
            int masterconfig_array[] = {37, 16, 8, 4, 2, 1};
        } else {
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
    if(input.cmdOptionExists("-mcs")) {
        string bitvalue_range = input.getCmdOption("-mcs");
        string bitvalue_lowerval = input.getCmdOption(bitvalue_range);
        int bitvalue_range_int = stoi(bitvalue_range);
        int bitvalue_lowerval_int = stoi(bitvalue_lowerval);
        userconfig.mcs_range = bitvalue_range_int;
        userconfig.mcs_lowerval = bitvalue_lowerval_int;
    }
    if(input.cmdOptionExists("-ndp_ol")) {
        double ndp_ol = stoi(input.getCmdOption("-ndp_ol"));
        userconfig.ndp_ol = ndp_ol;
    }
    
    int T; cin>>T;
    int inputcase;
    cin >> inputcase;
    // vector<int> config = {1, 0, 0, 0, 0, 0};
    int random_frequency = random(0, 3);
    createRUConfigs(userconfig.maxRU, userconfig);
    int totalProfit = 0;
    cout<<((int)configs.size())<<"\n";
    // for (int i = 0; i<configs.size(); i++){
    //     vector<int> &config = configs[i];
    //     if(random_frequency==1){
    //         // if(config)
    //         if(config[RU_242]>=1){
    //                 config[RU_242] -= 1;
    //         }else if(config[RU_106]>=2){
    //             config[RU_106] -= 2;
    //             config[RU_26] -= 1;
    //         }else if(config[RU_52]>=4){
    //             config[RU_52] -= 4;
    //             config[RU_26] -= 1;
    //         }else if(config[RU_26]>=17){
    //             config[RU_26] -= 17;
    //         }
    //     }else if(random_frequency==2){
    //         if(config[RU_484]>=1){
    //             config[RU_484] -= 1;
    //         }else if(config[RU_242]>=2){
    //                 config[RU_242] -= 2;
    //         }else if(config[RU_106]>=4){
    //             config[RU_106] -= 4;
    //         }else if(config[RU_52]>=16){
    //             config[RU_52] -= 16;
    //         }else if(config[RU_26]>=18){
    //             config[RU_26] -= 18;
    //         }
    //     }else if(random_frequency==3){

    //     }
    //     for (int j = 0; j < configs[i].size(); ++j){
    //         configs[i][j] = config[j];
    //     }
    // }
    // for (int i = 0; i<configs.size(); i++){
    //     for (int j = 0; j < configs[i].size(); ++j){
    //         cout<<configs[i][j]<<" ";
    //     }
    //     cout<<"\n";
    // }
        // Running the general case
    int timeperiod;
    double packetDropFactor;
    cin >> timeperiod;
    cin >> granularity;
    cin >> criticalThreshold;
    cin >> packetDropFactor;
    
    vector<vector<double>> producers; // to store input details for multiple runs of createBatch
    vector<Packet> currentPackets;
    // currentPackets.reserve(600000);
    /*
        we are assuming non Determinstic packets are all of same profit
        therefore counting the number of non Deterministic packets scheduled
        should be enough.

        nonDeterministicPackets stores the ndp coming per round
   */
    vector<vector<Packet>> nonDeterministicPackets(n_rounds);
    for (int roundNo = 0; roundNo < n_rounds; roundNo++)
    {
        cout<<roundNo<<" round:\n============\n";
        int start_time = roundNo * timeperiod;
        // Clean-up: remove if deadline is over
        auto itr = currentPackets.begin();
        while(itr != currentPackets.end()){
            Packet &pckt = (*itr);
            if(pckt.deadline <= start_time || pckt.scheduled==true){
                itr = currentPackets.erase(itr);
            } else {
                ++itr;
            }
        }

        // profit increases with each subsequent round for ndp packets
        for(Packet& pckt: currentPackets) 
            pckt.updateNDPacket(criticalThreshold);
        // Create the current deterministic batch

        stations_count = createBatch(currentPackets, start_time, timeperiod, inputcase, producers, criticalThreshold);

        cout<<"Batch created\n";

        if (roundNo - 2 >= 0)
        {
            for(Packet& pckt: nonDeterministicPackets[roundNo-2]){
                pckt.setNDP();
            }
            currentPackets.insert(currentPackets.end(),
                                  nonDeterministicPackets[roundNo - 2].begin(),
                                  nonDeterministicPackets[roundNo - 2].end());
        }
        // running the algorithm
        for(int ii = 0; ii < (int)currentPackets.size(); ii++){
            currentPackets[ii].id = ii;
        }
        vector<IntervalData> intervalData = LSDS(currentPackets, start_time, timeperiod, stations_count, criticalThreshold, granularity, packetDropFactor, userconfig);
        cout<<"LSDS algo executed\n";
        for(auto &itvl: intervalData){
            packets_scheduled2+=itvl.packets.size();
            for(auto &pckt: itvl.packets){
                pckt.scheduled = true;
                if(pckt.penalty >= critical_threshold){
                    critical_packets_scheduled++;
                }else{
                    noncritical_packets_scheduled++;
                }
                for(auto &pckt2: currentPackets){
                    if(pckt.id == pckt2.id){
                        pckt2.scheduled = true;
                        break;
                    }
                }
            }
        }
        //ndp packets are generated here
        createNDP(nonDeterministicPackets[roundNo], start_time, timeperiod, granularity, userconfig.ndp_ol);

        // Allocating to free RUs
        if (roundNo - 1 >= 0)
        {
            fitPacketsToFreeRUs(intervalData, nonDeterministicPackets[roundNo - 1], packetDropFactor, userconfig);
        }
        fitPacketsToFreeRUs(intervalData, nonDeterministicPackets[roundNo], packetDropFactor, userconfig);

        // Allocating to spaces with no intervals
        if(roundNo-1 < 0){
            vector<Packet> noPackets;
            fitPacketsToEmptySpaces(intervalData, noPackets, nonDeterministicPackets[roundNo], start_time, start_time + timeperiod, userconfig);
        } else {
            fitPacketsToEmptySpaces(intervalData, nonDeterministicPackets[roundNo - 1], nonDeterministicPackets[roundNo], 
                start_time, start_time + timeperiod, userconfig);
        }
        for(auto &interval_data: intervalData){
            totalProfit += interval_data.score;
        }
        cout<<"Allocated to free RUs and empty spaces\n";
    }
    cout<<"\n==========end of round simulation============\n";
    // totalnoncritical = total_packets - totalcritical;
    // cout<<ndp_first_bit<<" "<<ndp_last_bit<<" "<<ndp_data_transferred<<"\n";
    // cout<<"Packets scheduled: "<<packets_scheduled2+packets_scheduled1<<"\n";
    // cout<<"Packets dropped: "<<total_packets - (packets_scheduled1+packets_scheduled2)<<"\n";
    // cout<<"Total packets: "<<total_packets<<"\n";
    // cout<<"%age Critical packets dropped: "<<(((double)totalcritical - (double)critical_packets_scheduled)/totalcritical)*100<<"\n";
    // cout<<"%age Non critical packets dropped: "<<(((double)totalnoncritical - (double)noncritical_packets_scheduled)/totalnoncritical)*100<<"\n";
    // cout<<"Percentage of packets dropped: "<<(((double)total_packets - (double)(packets_scheduled1+packets_scheduled2))/total_packets)*100<<"\n";
    // cout<<"Total profit: "<<totalProfit<<"\n";
    cout<<n_rounds<<" "<<timeperiod<<endl;
    cout<<"NDP throughput: "<<((double)((ndp_data_transferred-9000)*8*1000)/((n_rounds*((double)timeperiod/granularity))*1024*1024))<<" Mb/s\n";
    cout<<"Transmission time given to NDP: "<<ndp_transmission_time<<" nunits\n";
    cout<<"Resource (Tonnes) given to NDP: "<<ndp_resource_allocated<<"\n";
}
/*
Input:
*/
