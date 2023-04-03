#include <iostream>
#include <set>
#include <vector>
#include <queue>
#include <cmath>
#include <stack>
#include <algorithm>
#include <chrono>
// #include <lemon/smart_graph.h>
// #include <lemon/network_simplex.h>
// #include <lemon/preflow.h>

using namespace std;
using namespace std::chrono;
// using namespace lemon;

// using NS = NetworkSimplex<SmartDigraph, int, int>;

// template<typename ValueType>
// using ArcMap = SmartDigraph::ArcMap<ValueType>;

struct Interval{
    int start; int end;
};

class Packet{
    public:
    int penalty, arrival, type, startTime, datasize;
    double deadline;
    Packet(){
        datasize = penalty = arrival = deadline = type = startTime = -1;
    }
    Packet(int penalty, int arrival, int datasize, double deadline, int type){
        this->penalty = penalty;
        this->arrival = arrival;
        this->datasize = datasize;
        this->deadline = deadline; 
        this->startTime = -1;
    }
};

//------------------- Interval Generation -------------------

const int RU_484 = 0;
const int RU_242 = 1;
const int RU_106 = 2;
const int RU_52 = 3;
const int RU_26 = 4;

double calcTxop(double size, int mode){
    double bitrate;
    if(mode == RU_26) bitrate = 11.1;
    else if(mode == RU_52) bitrate = 22.2;
    else if(mode == RU_106) bitrate = 47.2;
    else if(mode == RU_242) bitrate = 121.9;
    else if(mode == RU_484) bitrate = 243.8;
    else bitrate = -1;

    double res = (size * 8.0)/(bitrate * 1000.0);
    return res;
}

vector<vector<int>> configs;
void createRUConfigs(int curr, vector<int>& config){
    //Print
    vector<int> tempfigs(config);
    configs.push_back(tempfigs);
    //try divisions
    for(int times = 1; times <= config[curr]; times++){
        config[curr] -= times;
        if(curr == RU_484){
            config[RU_242] += times * 2;
            createRUConfigs(curr+1, config);
            config[RU_242] -= times * 2;
        } else if(curr == RU_242){
            config[RU_106] += 2*times;
            config[RU_26] += times;
            createRUConfigs(curr+1, config);
            config[RU_26] -= times;
            config[RU_106] -= 2*times;
        } else if(curr == RU_106){
            config[RU_52] += 2*times;
            createRUConfigs(curr+1, config);
            config[RU_52] -= 2*times;
        } else if(curr == RU_52){
            config[RU_26] += 2*times;
            createRUConfigs(curr+1, config);
            config[RU_26] -= 2*times;
        } else if(curr == RU_26){
            //cannot divide more
        }
        config[curr] += times;
    }
}

//min priority queue
bool operator<(const Packet& p1, const Packet& p2){
    return p1.penalty > p2.penalty;
}

bool packetMatchesInterval(int intervalStart, int intervalEnd, Packet& pckt, int mode){
    double transmission_dur = calcTxop(pckt.datasize, mode);
    if(pckt.arrival <= intervalStart && intervalStart + transmission_dur <= min((double)intervalEnd, pckt.deadline)) return true;
    return false;
}

class IntervalData{
public:
    vector<int> config;
    vector<int> freeslots;
    int start; int end;
    vector<Packet> packets;
    int score;
    IntervalData(int start, int end):start(start),end(end),score(0){
        freeslots.assign(5,0);
        config.assign(5,0);
    }
    void updateFreeSlots(int mode, int val){
        freeslots[mode] = val;
    }
    void setConfig(vector<int>& config){
        for(int i = 0; i < 5; i++){
            this->config[i] = config[i];
        }
    }
    void insert(Packet& pckt){
        packets.push_back(pckt);
        score += pckt.penalty; 
    }
};

// ----------------------------------------------------------
// -------------- Heuristic Algorithm -----------------------
// ----------------------------------------------------------

void selectPacketsForConfig(IntervalData& currInterval, vector<Packet>& availablePackets, vector<Packet>& newAvailablePackets, vector<int>& config, int& prevscore){
    priority_queue<Packet> topPackets[5];
    int itvl_start = currInterval.start;
    int itvl_end = currInterval.end;
    int curscore = 0;
    stack<Packet> remainingPacketsStack;
    for(Packet& pckt : availablePackets){
        bool assigned = false;
        for(int mode = RU_26; mode >= RU_484; mode--){
            if(packetMatchesInterval(itvl_start, itvl_end, pckt, mode) && config[mode] > 0){
                if(topPackets[mode].size() < config[mode]){
                    curscore += pckt.penalty;
                    topPackets[mode].push(pckt);
                    assigned = true; break;
                }
                else if(topPackets[mode].top().penalty < pckt.penalty){
                    Packet rempckt = topPackets[mode].top(); topPackets[mode].pop();
                    curscore -= rempckt.penalty;
                    curscore += pckt.penalty;
                    remainingPacketsStack.push(rempckt);
                    // newAvailablePackets.push_back(rempckt);
                    topPackets[mode].push(pckt);
                    assigned = true; break;
                }
            }
        }
        if(!assigned){
            remainingPacketsStack.push(pckt);
            // newAvailablePackets.push_back(pckt);
        }
    }
    //can't assign packets
    if(curscore < prevscore) return;

    prevscore = curscore;
    //reset stuff
    currInterval.setConfig(config);
    currInterval.packets.clear();
    currInterval.score = 0;
    newAvailablePackets.clear();

    while(!remainingPacketsStack.empty()){
        newAvailablePackets.push_back(remainingPacketsStack.top()); 
        remainingPacketsStack.pop();
    }
    for(int mode = RU_484; mode <= RU_26; mode++){
        currInterval.updateFreeSlots(mode, config[mode] - (int)topPackets[mode].size());
        while(!topPackets[mode].empty()){
            Packet tempckt = topPackets[mode].top(); topPackets[mode].pop();
            currInterval.insert(tempckt);
        }
    }
}


void createInterval(vector<Packet>& packets, int T){
    const double alpha = 2;
    vector<Packet> availablePackets(packets);
    vector<IntervalData> selectedIntervals;
    int delta = 6;
    int totalval = 0;
    for(Packet& pckt: packets){
        totalval += pckt.penalty;
    }
    for(int d = 1; d <= delta; d++){
        for(int t = 0; t < T-d; t++){
            int prevscore = -1;
            int intervalStart = t, intervalEnd = t+d;
            IntervalData currInterval(intervalStart, intervalEnd);
            priority_queue<Packet> topPackets;
            vector<Packet> newAvailablePackets;
            vector<IntervalData> newSelectedIntervals;
            for(vector<int>& config: configs){
                selectPacketsForConfig(currInterval, availablePackets, newAvailablePackets, config, prevscore);
            }
            //overlapping intervals
            vector<IntervalData> overlapIntervals;
            vector<IntervalData> remainingIntervals;
            double overlapScore = 0;
            for(IntervalData& interval: selectedIntervals){
                if(!(interval.end <= currInterval.start || interval.start >= currInterval.end)){
                    overlapIntervals.push_back(interval);
                    overlapScore += interval.score;
                } else {
                    remainingIntervals.push_back(interval);
                }
            }
            if(alpha * (double)overlapScore < currInterval.score){
                //choose curr interval
                remainingIntervals.push_back(currInterval);
                swap(selectedIntervals, remainingIntervals);

                for(IntervalData& itvl: overlapIntervals){
                    for(Packet& pckt: itvl.packets){
                        newAvailablePackets.push_back(pckt);
                    }
                }
            } else {
                // reject curr interval
                for(Packet& pckt: currInterval.packets){
                    newAvailablePackets.push_back(pckt);
                }
            }
            swap(availablePackets, newAvailablePackets);
        }
    }

    //final sweep:
    for(Packet& pckt: availablePackets){
        for(IntervalData& itvl: selectedIntervals){
            for(int mode = RU_26; mode >= RU_484; mode--){
                if(itvl.freeslots[mode] <= 0) continue;
                if(packetMatchesInterval(itvl.start, itvl.end, pckt, mode)){
                    itvl.freeslots[mode]--;
                    itvl.insert(pckt); break;
                }
            }
        }
    }
    int score = 0;
    for(IntervalData& itvl: selectedIntervals){
        Interval tempitvl;
        score += itvl.score;
    }
    cout<<"Heuristic Score: ";
    cout<<score<<"\n";
}
// ----------------------------------------------------------
// -----------------------BASELINES-------------------------
bool edf(Packet& p1, Packet& p2){
    if(p1.deadline == p2.deadline){
        return p1.arrival < p2.arrival;
    }
    return p1.deadline < p2.deadline;
}
bool lrf(Packet& p1, Packet& p2){
    double wt1 = p1.deadline/(double)p1.penalty;
    double wt2 = p2.deadline/(double)p2.penalty;
    if(wt1 == wt2){
        return p1.arrival < p2.arrival;
    }
    return wt1 < wt2;
}

#define EDF 1
#define LRF 2

void newBaseline(vector<Packet> packets, int T, int opt){
    if(opt == EDF){
        sort(packets.begin(), packets.end(), edf);
    } else if(opt == LRF){
        sort(packets.begin(), packets.end(), lrf);
    } 
    int score = 0;
    int lastend = 0;
    int lenPackets = packets.size();
    for(int i = 0; i < lenPackets;){
        //try all arrival packets at start
        if(lastend >= packets[i].deadline){
            i++; continue;
        }
        int maxscore = -1;
        int packetsDropped = 0;
        vector<int> cfig(0);
        int newend = lastend;
        int newstart = max(lastend, packets[i].arrival);
        //iterate through all configurations
        for(vector<int>& config: configs){
            int m = config[RU_484] + config[RU_242] + config[RU_106] + config[RU_52] + config[RU_26];
            int windowsize = 0;
            int curscore = 0;
            int curdrops = 0;
            //try to fit first m packets if possible
            for(int j = i; j < min(i+m, lenPackets); j++){
                Packet& pckt = packets[j];
                if(pckt.arrival > newstart) continue;
                bool assigned = false;
                for(int mode = RU_26; mode >= RU_484; mode--){
                    if(config[mode] && calcTxop(pckt.datasize, mode)+newstart <= pckt.deadline){
                        assigned = true;
                        windowsize = max(windowsize, (int)ceil(calcTxop(pckt.datasize, mode)));
                        break;
                    }
                }
                if(assigned){
                    curscore += pckt.penalty;
                    packetsDropped++;
                }
            }
            //see if this score is better than the previous ones
            if(curscore > maxscore || (curscore == maxscore && newend > newstart + windowsize)){
                cfig = config;
                newend = newstart + windowsize;
                maxscore = curscore;
                packetsDropped = curdrops;
            }
        }
        lastend = newend;
        if(newend >= T) break;
        //update max score
        score += max(maxscore,0);
        // -----
        // -----
        if(cfig.size() == 0) i += 1;
        else i += cfig[RU_484] + cfig[RU_242] + cfig[RU_106] + cfig[RU_52] + cfig[RU_26]; 
    }
    //output
    if(opt == EDF){
        cout<<"New EDF Score: "<<score<<"\n";
    } else if(opt == LRF){
        cout<<"New LRF Score: "<<score<<"\n";
    } else {
        cout<<"NLRF Score: "<<score<<"\n";
    }
}
void baseline(vector<Packet> packets, int T, int opt){
    if(opt == EDF){
        sort(packets.begin(), packets.end(), edf);
    } else if(opt == LRF){
        sort(packets.begin(), packets.end(), lrf);
    } 
    int score = 0;
    int lastend = 0;
    int lenPackets = packets.size();
    for(int i = 0; i < lenPackets;){
        //try all arrival packets at start
        int prevscore = -1;
        vector<int> cfig;
        int newend = lastend;
        for(vector<int>& config: configs){
            int m = config[RU_484] + config[RU_242] + config[RU_106] + config[RU_52] + config[RU_26];
            int lastDeadline = (int)floor(packets[min(i+m-1, lenPackets-1)].deadline);
            for(int newstart = lastend; newstart <= min(lastDeadline, T-1); newstart++){
                int windowsize = 0;
                int curscore = 0;
                for(int j = i; j < min(i+m, lenPackets); j++){
                    Packet& pckt = packets[j];
                    if(pckt.arrival > newstart) continue;
                    curscore += pckt.penalty;
                    if(config[RU_26] && calcTxop(pckt.datasize, RU_26)+newstart <= pckt.deadline){
                        windowsize = max(windowsize, (int)ceil(calcTxop(pckt.datasize, RU_26)));
                    }
                    else if(config[RU_52] && calcTxop(pckt.datasize, RU_52)+newstart <= pckt.deadline){
                        windowsize = max(windowsize, (int)ceil(calcTxop(pckt.datasize, RU_52)));
                    }
                    else if(config[RU_106] && calcTxop(pckt.datasize, RU_106)+newstart <= pckt.deadline){
                        windowsize = max(windowsize, (int)ceil(calcTxop(pckt.datasize, RU_106)));
                    }
                    else if(config[RU_242] && calcTxop(pckt.datasize, RU_242)+newstart <= pckt.deadline){
                        windowsize = max(windowsize, (int)ceil(calcTxop(pckt.datasize, RU_242)));
                    } 
                    else if(config[RU_484] && calcTxop(pckt.datasize, RU_484)+newstart <= pckt.deadline){
                        windowsize = max(windowsize, (int)ceil(calcTxop(pckt.datasize, RU_484)));
                    } else {
                        curscore -= pckt.penalty;
                    }
                }
                if(curscore > prevscore || (curscore == prevscore && newend > newstart + windowsize)){
                    cfig = config;
                    newend = newstart + windowsize;
                    prevscore = curscore;
                }
            }
        }
        if(newend >= T) break;
        lastend = newend;
        score += max(prevscore,0);
        if(cfig.size() == 0) i += 1;
        else i += cfig[RU_484] + cfig[RU_242] + cfig[RU_106] + cfig[RU_52] + cfig[RU_26]; 
    }
    //output
    if(opt == EDF){
        cout<<"EDF Score: "<<score<<"\n";
    } else if(opt == LRF){
        cout<<"LRF Score: "<<score<<"\n";
    } else {
        cout<<"NLRF Score: "<<score<<"\n";
    }
}
// ----------------------------------------------------------

void takeInput(int T, vector<Packet>& packets){
    int stations; cin>>stations;
    vector<vector<double>> producers(stations, vector<double>(5));
    // Time period | size | delay | penalty | nodes
    for(int i = 0; i < stations; i++){
        for(int j = 0; j < 5; j++){
            cin>>producers[i][j];
        }
    }
    for(auto &pro: producers){
        int Tp = (int)pro[0], size = (int)pro[1];
        double delay = pro[2];
        int penalty = (int)pro[3], nodes = (int)pro[4];
        for(int arri = 0; arri < T; arri += Tp){
            for(int ctr = 0; ctr < nodes; ctr++){
                Packet pckt(penalty, arri, size, arri*1.0 + delay, 0);
                packets.push_back(pckt);
            }
        }
    }
}

int main(){
    // ios::sync_with_stdio(false);
    // cin.tie(0);
    int T; cin>>T;
    vector<int> config = {1,0,0,0,0};
    createRUConfigs(0, config);
    while(T--){
        int timeperiod; cin>>timeperiod;
        vector<Packet> packets(0);
        takeInput(timeperiod, packets);
        cout<<"---------\n";
        int maxScorePossible = 0;
        for(Packet& pckt: packets){
            maxScorePossible += pckt.penalty;
        }
        cout<<"Total Profit : "<< maxScorePossible<<"\n";
        auto start = high_resolution_clock::now();
        createInterval(packets, timeperiod);
        auto t1 = high_resolution_clock::now();
        newBaseline(packets, timeperiod, EDF);
        baseline(packets, timeperiod, EDF);
        newBaseline(packets, timeperiod, LRF);
        baseline(packets, timeperiod, LRF);
        auto dur1 = duration_cast<microseconds>(t1 - start);
        // auto dur2 = duration_cast<microseconds>(t2 - t1);
        cout<<"Heuristic algo: "<<(dur1.count())/1000<<" milliseconds\n";
        cout<<"---------\n";
    }
    // createRUConfigs(0,config);
}

/*
Input:
n packets, x intervals
each interval corresponds to m RUs so one interval can take m packets (right?)
1 source, 1 sink, n packets, x intervals
source --1-- packets
packets --1-- intervals
intervals --m-- sink
the profits for all edges other than src --1-- packets are zero (no extra profit)
1
3 23
5
10 1 1 10
5 1 1 2
6 1 1 3
7 1 1 2
12 3 10 15
Meeting agenda:
- discuss how the packets are generated
onDemandApplication 
simulation packet generation band 
- how to handle non integral 
LRF
NLRF
EDF (earliest deadline first)
*/