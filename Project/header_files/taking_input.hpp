#pragma once

#include <iostream>
#include <vector>
#include "Packet.hpp"
#include <ctime>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;

int universalCounter = 0;
int ndpProfit, ndpDelay;
int total_packets = 0;
int totalcritical = 0;
int critical_threshold = 0;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
  // mt19937 rng((uint64_t) new char);
int random(int range_from, int range_to) {
	if(range_from == range_to) return range_to;
	int temp = (rng())%(range_to - range_from + 1);
	temp += range_from;
	return temp;
}
void takeInput(vector<vector<double>> &producer, int size){
    int stations; cin>>stations;
    producer.resize(stations, vector<double>(size));
    for(int i = 0; i < stations; ++i){ //usecase 1
        for(int j = 0; j < size; ++j){
            cin>>producer[i][j];
        }
    }
    int minProfitNDP;
    cin>>minProfitNDP;
    ndpProfit = minProfitNDP;
    int minDelayNDP;
    cin>>minDelayNDP;
    ndpDelay = minDelayNDP;
}
//does not support granularity yet.
int packetsGeneratorPoisson(int T, vector<Packet>& packets, int start_time, vector<vector<double>> &producers){
    int stations = (int)producers.size();
    vector<int> pcktsize(stations);
    vector<int> pcktdelay(stations);
    vector<int> pcktpenalty(stations);
    vector<int> pcktnodes(stations);
    //size | delay | penalty | nodes
    // cout<<"Size | Delay | penalty | nodes\n";
    for (int i = 0; i < stations; ++i){
        pcktsize[i] = (int)producers[i][0];
        pcktdelay[i] = (int)producers[i][1];
        pcktpenalty[i] = (int)producers[i][2];
        pcktnodes[i] = (int)producers[i][3];
    }

    std::random_device rd; // uniformly-distributed integer random number generator
    std::mt19937 rang (rd ()); // mt19937: Pseudo-random number generation

    double averageArrival = 1;  //in nunits
    double lamda = 1 / averageArrival;
    std::exponential_distribution<double> exp (lamda);
    int n_stations = 0; // count of stations
    for(int st = 0; st < stations; st++){
        double sumArrivalTimes=0;
        double newArrivalTime;
        while(sumArrivalTimes < T)
        {
            newArrivalTime=  exp.operator() (rang);// generates the next random number in the distribution 
            sumArrivalTimes  = sumArrivalTimes + newArrivalTime;  
            for(int i = 0; i < pcktnodes[st]; i++){
                Packet pckt(pcktpenalty[st], sumArrivalTimes+start_time, pcktsize[st], sumArrivalTimes+start_time+pcktdelay[st], n_stations+i, universalCounter++, ((double)random(0, 1000)/1000));
                packets.push_back(pckt);
                // cout << "schedule hua\n";
                if(pckt.penalty >= critical_threshold){
                    totalcritical++;
                }
                total_packets++;
            }
            // j += (int)pcktnodes.size();
        }
        n_stations += (int) pcktnodes[st];
    }
    return n_stations;
}
int uniformInput(int T, vector<Packet>& packets, int start_time, vector<vector<double>> &producers){
    
    int n_stations = 0;  // count of stations
    for(auto &pro: producers){
        //print pro
        double Tp = pro[0];
        int size = (int)pro[1]; //general
        double delay = pro[2]; //general
        int penalty = (int)pro[3]; //general
        int nodes = (int)pro[4]; //general 
        for(double arri = start_time; arri < (double)T+start_time; arri += Tp){
            for(int ctr = 0; ctr < nodes; ++ctr){
                Packet pckt(penalty, arri, size, arri + delay, n_stations+ctr, universalCounter++, 1);
                packets.push_back(pckt);
                if(pckt.penalty >= critical_threshold){
                    totalcritical++;
                }
                total_packets++;
            }
        }
        n_stations += nodes;
    }
    return n_stations;
}
int usecase1Input(int T, vector<Packet>& packets, int start_time, vector<vector<double>> &producers){
    int n_stations = 0; // count of stations
    for(auto &pro: producers){
        //print pro
        double Tp = pro[0]; 
        int minsize = (int)pro[1]; //usecase 1
        int maxsize = (int)pro[2]; //usecase 1
        double delay = pro[3]; 
        int penalty = (int)pro[4], nodes = (int)pro[5];
        for(double arri = start_time; arri < (double)T+start_time; arri += Tp){
            for(int ctr = 0; ctr < nodes; ++ctr){
                Packet pckt(penalty, arri, random(minsize, maxsize), arri + delay, n_stations+ctr, universalCounter++, 1);
                if(pckt.penalty >= critical_threshold){
                    totalcritical++;
                }
                packets.push_back(pckt);
                total_packets++;
            }
        }
        n_stations += nodes;
    }
    return n_stations;
}
int usecase2Input(int T, vector<Packet>& packets, int start_time, vector<vector<double>> &producers){
    int n_stations = 0; // count of stations
    for(auto &pro: producers){
        //print pro
        double meanArrRate = pro[0]; 
        int pcktPerReqlow = (int)pro[1]; //usecase 2
        int pcktPerReqhigh = (int)pro[2]; //usecase 2
        int size = (int)pro[3]; //usecase 2
        double delay = pro[4];
        double Tp = 1000/(meanArrRate*random(pcktPerReqlow, pcktPerReqhigh));
        int penalty = (int)pro[5], nodes = (int)pro[6];
        for(double arri = start_time; arri < (double)T+start_time; arri += Tp){
            for(int ctr = 0; ctr < nodes; ++ctr){
                Packet pckt(penalty, arri, 10, arri + delay, n_stations+ctr, universalCounter++, 1);
                if(pckt.penalty >= critical_threshold){
                    totalcritical++;
                }
                packets.push_back(pckt);
                total_packets++;
            }
        }
        n_stations += nodes;
    }
    return n_stations;
}
int usecase3Input(int T, vector<Packet>& packets, int start_time, vector<vector<double>> &producers){
    vector<vector<double>> producer;
    int n_stations = 0;
    for(auto &pro: producers){
        //print pro
        double bandwidth = pro[0]; 
        int size = (int)pro[1];
        double delay = pro[2];
        int penalty = (int)pro[3], nodes = (int)pro[4];
        double Tp = (size*8.0)/((double)bandwidth);
        // cout<<Tp<<"\n";
        for(double arri = start_time; arri < (double)T+start_time; arri += Tp){
            for(int ctr = 0; ctr < nodes; ++ctr){
                Packet pckt(penalty, arri, size, arri + delay, n_stations+ctr, universalCounter++, 1);
                if(pckt.penalty >= critical_threshold){
                    totalcritical++;
                }
                packets.push_back(pckt);
                ++total_packets;
            }
        }
        n_stations += nodes;
    } 
    return n_stations;
}

int ndpPacketGenerator(int timeperiod, vector<Packet> &currentNdpPackets, int start_time, int granularity, int offeredLoad){
    int stations = 3;
    int nodes = 1;
    // (1/granularity)ms = 1 nunit

    // std::mt19937 rang (42); // mt19937: Pseudo-random number generation, fixed seed 42

    // double averageArrival = 0.5;  //in nunits
    // double lamda = 1 / averageArrival;
    // std::exponential_distribution<double> exp (lamda);
    // std::uniform_int_distribution<int> intdist(200,1500); // random number gen between 200 and 1500
    int n_stations = 0; // count of stations
    vector<Packet> newNdpPackets(0);
    double offered_load = (((double)offeredLoad*1024*1024))/(1000*stations*granularity);
    double datasize = 1500.0;
    double total_size=0;
    cout<<"offered load: "<<offered_load<<endl;
    for(int st = 0; st < stations; st++){
        if(offered_load > 0){
            double sumArrivalTimes=0;
            double newArrivalTime;
            double Tp = ((double)datasize*(8.0))/((double)offered_load);
            // cout<<"time period: "<<Tp<<endl;
            while(sumArrivalTimes < timeperiod){
                // newArrivalTime =  exp.operator() (rang);// generates the next random number in the distribution 
                sumArrivalTimes  = sumArrivalTimes + Tp;
                for(int i = 0; i < nodes; i++){
                    // int datasize = 1500; // between 200 and 1500 bytes
                    Packet pckt(ndpProfit, sumArrivalTimes+start_time, datasize, sumArrivalTimes+ndpDelay+start_time, n_stations+i, universalCounter++, 1);
                    total_size+=datasize;
                    if(pckt.penalty >= critical_threshold){
                        totalcritical++;
                    }
                    newNdpPackets.push_back(pckt);
                    total_packets++;
                }
                // j += (int)pcktnodes.size();
            }
            n_stations += nodes;
        }
    }
    // cout<<"total ndp size:"<<(double)total_size/(1024*1024)<<endl;
    sort(newNdpPackets.begin(), newNdpPackets.end(), [](Packet& p1, Packet& p2){
        return p1.arrival < p2.arrival;
    });
    for(Packet& pckt: newNdpPackets){
        currentNdpPackets.push_back(pckt);
    }
    return n_stations;
    
    
    
    // int stations = 3;
    // int nodes = 1;

    // std::mt19937 rang (42); // mt19937: Pseudo-random number generation, fixed seed 42

    // double averageArrival = 2;  //in nunits
    // double lamda = 1 / averageArrival;
    // std::exponential_distribution<double> exp (lamda);
    // std::uniform_int_distribution<int> intdist(200,1500); // random number gen between 200 and 1500
    // int n_stations = 0; // count of stations
    // vector<Packet> newNdpPackets(0);
    // for(int st = 0; st < stations; st++){
    //     double sumArrivalTimes=0;
    //     double newArrivalTime;
    //     while(sumArrivalTimes < timeperiod)
    //     {
    //         newArrivalTime=  exp.operator() (rang);// generates the next random number in the distribution 
    //         sumArrivalTimes  = sumArrivalTimes + newArrivalTime;  
    //         for(int i = 0; i < nodes; i++){
    //             int datasize = intdist(rang); // between 200 and 1500 bytes
    //             Packet pckt(ndpProfit, sumArrivalTimes+start_time, datasize, sumArrivalTimes+ndpDelay+start_time, n_stations+i, universalCounter++, ((double)random(0, 1000)/1000));
    //             if(pckt.penalty >= critical_threshold){
    //                 totalcritical++;
    //             }
    //             newNdpPackets.push_back(pckt);
    //             total_packets++;
    //         }
    //         // j += (int)pcktnodes.size();
    //     }
    //     n_stations += nodes;
    // }
    // sort(newNdpPackets.begin(), newNdpPackets.end(), [](Packet& p1, Packet& p2){
    //     return p1.arrival < p2.arrival;
    // });
    // for(Packet& pckt: newNdpPackets){
    //     currentNdpPackets.push_back(pckt);
    // }
    // return n_stations;
}

void createNDP(vector<Packet> &currentNdpPackets, int start_time, int timeperiod, int granularity, int offeredLoad){
    ndpPacketGenerator(timeperiod, currentNdpPackets, start_time, granularity, offeredLoad);
}

int createBatch(vector<Packet> &currentPackets, int start_time, int timeperiod, int input_type, vector<vector<double>> &producers, int criticalThreshold){
    int stations;
    critical_threshold = criticalThreshold;
    if(producers.size()==0){
        takeInput(producers, (input_type == 0 ? 5 : (input_type == 1 ? 6 : (input_type == 2 ? 7 : (input_type == 3 ? 5 : (input_type == 4 ? 4 : 0))))));
    }
    switch (input_type){
        case 0: stations = uniformInput(timeperiod, currentPackets, start_time, producers); break;
        case 1: stations = usecase1Input(timeperiod, currentPackets, start_time, producers); break;
        case 2: stations = usecase2Input(timeperiod, currentPackets, start_time, producers); break;
        case 3: stations = usecase3Input(timeperiod, currentPackets, start_time, producers); break;
        case 4: stations = packetsGeneratorPoisson(timeperiod, currentPackets, start_time, producers); break;
        default: stations = 0;
    }
    return stations; // basically station count
}