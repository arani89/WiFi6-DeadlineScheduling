#pragma once

#include <iostream>
#include <vector>
#include "Packet.hpp"
#include "commonDefines.hpp"

using namespace std;

class IntervalData{
public:
    vector<int> config;
    vector<int> freeslots;
    int start; int end;
    vector<Packet> packets;
    vector<vector<int>> best_config_packets;
    int score;
    IntervalData(int start, int end, int size):start(start),end(end),score(0){
        freeslots.assign(7,0);
        config.assign(7,0);
    }
    void updateFreeSlots(int mode, int val){
        freeslots[mode] = val;
    }
    void setConfig(vector<int>& config){
        for(int i = 0; i < 7; i++){
            this->config[i] = config[i];
        }
    }
    void insert(Packet& pckt){
        packets.push_back(pckt);
        score += pckt.penalty; 
    }
    int getavailfreq(){
        return (
            freeslots[RU_1992]*1992 +
            freeslots[RU_996]*996 + 
            freeslots[RU_484]*484 + 
            freeslots[RU_242]*242 + 
            freeslots[RU_106]*106 + 
            freeslots[RU_52]*52 +
            freeslots[RU_26]*26
        );
    }
    void printstuff(){
        int totfreeslots = 0;
        for(auto val: freeslots){
            totfreeslots += val;
        }
        if(totfreeslots == 0) return;
        cout<<"Free slots = "<<totfreeslots<<"\n";
        if(freeslots[RU_1992] > 0) cout<<"RU_1992 = "<<freeslots[RU_1992]<<", ";
        if(freeslots[RU_996] > 0) cout<<"RU_996 = "<<freeslots[RU_996]<<", ";
        if(freeslots[RU_484] > 0) cout<<"RU_484 = "<<freeslots[RU_484]<<", ";
        if(freeslots[RU_242] > 0) cout<<"RU_242 = "<<freeslots[RU_242]<<", ";
        if(freeslots[RU_106] > 0) cout<<"RU_106 = "<<freeslots[RU_106]<<", ";
        if(freeslots[RU_52] > 0) cout<<"RU_52 = "<<freeslots[RU_52]<<", ";
        if(freeslots[RU_26] > 0) cout<<"RU_26 = "<<freeslots[RU_26]<<", ";
        cout<<"\n";
        cout<<"score = "<< score<<"\n";
        cout<<"Start - End = "<<start<<" - "<<end<<"\n";
        cout<<"Config : ";
        for(int i = 0; i < 7; ++i){
            cout<<config[i]<<" ";
        }
        cout<<"\n";
        // cout<<"Packets : \n";
        // for(Packet pckt: packets){
        //     cout<<pckt.arrival<<" "<<pckt.deadline<<" "<<pckt.penalty<<"\n";
        // }
    }
};