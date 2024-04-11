#pragma once

#include <iostream>
#include "commonDefines.hpp"

/*
    penalty: same as profit of the packet
    stationId: specifies which station the packet came from
    id: is a unique identifier
    ndp: boolean to see if a packet is non-deterministic or not
*/
class Packet{
    public:
    int penalty, stationId, id, client_id;
    double datasize;
    double arrival;
    double deadline;
    double packetDropFactor;
    bool ndp, scheduled;
    int mode;
    double timeAssigned;
    

	// default constructor
    Packet(){
        penalty = stationId = -1;
        datasize = -1;
        deadline = arrival = -1;
        id = 0; ndp = false;
        scheduled = false;
        mode = -1; timeAssigned = -1;
    }
	// initialize penalty, arrival, datasize, deadline, stationId, unique id
    Packet(int penalty, double arrival, double datasize, double deadline, int stationId, int id, int client_id, double packetDropFactor){
        this->penalty = penalty;
        this->arrival = arrival;
        this->datasize = datasize;
        this->deadline = deadline; 
        this->stationId = stationId;
        this->id = id;
        this->ndp = false;
        this->scheduled = false;
        mode = -1; timeAssigned = -1;
        this->client_id = client_id;
        this->packetDropFactor = packetDropFactor;
    }
	void setNDP(){ this->ndp = true; }
    void updateNDPacket(int critical_threshold){
        if(ndp == false) return;
        penalty = (penalty + critical_threshold)/2;
    }

    void setScheduled(bool val){ this->scheduled = val; }
    bool isScheduled(){ return this->scheduled; }

    void printstuff(){
        std::cout<<"arrival-deadline : "<< arrival<<" - "<<deadline<<"\n";
        std::cout<<"penalty: "<<penalty<<"\n\n";
    }
};

void schedulePacket(Packet& pckt, int mode, double transmission_time){
    pckt.mode = mode;
    pckt.timeAssigned = transmission_time;
    // pckt.scheduled = true;
}

// to transmit deterministic packet
void transmitDPacket(Packet& pckt){
    // can do stuff here
}

// to transmit non-deterministic packet
void transmitNDPacket(Packet& pckt, double start, double end){
    if(pckt.ndp == false) return;
    ndp_transmission_time += (double)pckt.timeAssigned;
    // std::cout << "\n"<<pckt.timeAssigned << " is the time assigned to this packet\n";
    ndp_data_transferred += pckt.datasize;
    ndp_resource_allocated += (pckt.mode == RU_1992) ? 1992 :(pckt.mode == RU_996) ? 996 : ((pckt.mode == RU_484) ? 484 : 
        ((pckt.mode == RU_242)? 242 : ((pckt.mode == RU_106)? 106 : 
        ((pckt.mode == RU_52)? 52: 26))));

    if(ndp_first_bit == -1){
        ndp_first_bit = start;
    }

    ndp_first_bit = std::min(ndp_first_bit, start);
    ndp_last_bit = std::max(ndp_last_bit, end);
}