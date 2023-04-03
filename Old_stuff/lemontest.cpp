#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <chrono>
#include <lemon/smart_graph.h>
#include <lemon/network_simplex.h>
#include <lemon/preflow.h>

using namespace std;
using namespace std::chrono;
using namespace lemon;

using NS = NetworkSimplex<SmartDigraph, int, int>;

template<typename ValueType>
using ArcMap = SmartDigraph::ArcMap<ValueType>;

struct Interval{
    int start; int end;
};

class Packet{
    public:
    int penalty, arrival, type, startTime;
    double transmission_dur;
    double deadline;
    Packet(){
        penalty = arrival = transmission_dur = deadline = type = startTime = -1;
    }
    Packet(int penalty, int arrival, double transmission_dur, double deadline, int type){
        this->penalty = penalty;
        this->arrival = arrival;
        this->transmission_dur = transmission_dur;
        this->deadline = deadline; 
        this->startTime = -1;
    }
    void print(){
        cout<<arrival<<" "<<transmission_dur<<" "<<penalty<<"\n";
    }
};

//------------------- Interval Generation -------------------

//min priority queue
bool operator<(const Packet& p1, const Packet& p2){
    return p1.penalty > p2.penalty;
}

bool packetMatchesInterval(int intervalStart, int intervalEnd, Packet& pckt){
    if(pckt.arrival <= intervalStart && intervalStart + pckt.transmission_dur <= min((double)intervalEnd, pckt.deadline)) return true;
    return false;
}

class IntervalData{
public:
    int start; int end;
    vector<Packet> packets;
    int score;
    IntervalData(int start, int end):start(start),end(end),score(0){}
    void insert(Packet& pckt){
        packets.push_back(pckt);
        score += pckt.penalty; 
    }
};

// ----------------------------------------------------------
// -------------- Heuristic Algorithm -----------------------
// ----------------------------------------------------------
vector<Interval> createInterval(vector<Packet>& packets, int T, int m = 1){
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
            int intervalStart = t, intervalEnd = t+d;
            IntervalData currInterval(intervalStart, intervalEnd);
            priority_queue<Packet> topPackets;
            vector<Packet> newAvailablePackets;
            vector<IntervalData> newSelectedIntervals;
            for(Packet& pckt : availablePackets){
                if(packetMatchesInterval(intervalStart, intervalEnd, pckt)){
                    //choosing top m packets
                    if(topPackets.size() < m){
                        topPackets.push(pckt);
                    } 
                    else if(topPackets.top().penalty < pckt.penalty){
                        Packet rempckt = topPackets.top(); topPackets.pop();
                        newAvailablePackets.push_back(rempckt);
                        topPackets.push(pckt);
                    } else {
                        newAvailablePackets.push_back(pckt);
                    }
                } else {
                    newAvailablePackets.push_back(pckt);
                }
            }
            while(!topPackets.empty()){
                Packet pckt = topPackets.top(); topPackets.pop();
                currInterval.insert(pckt);
            }
            cout<<currInterval.score<<"\n";
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
            if(alpha * overlapScore < currInterval.score){
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
    vector<Interval> partition;
    for(Packet& pckt: availablePackets){
        for(IntervalData& itvl: selectedIntervals){
            if(packetMatchesInterval(itvl.start, itvl.end, pckt) && itvl.packets.size() < m){
                itvl.insert(pckt); break;
            }
        }
    }
    int score = 0;
    for(IntervalData& itvl: selectedIntervals){
        Interval tempitvl;
        tempitvl.start = itvl.start; tempitvl.end = itvl.end;
        score += itvl.score;
        partition.push_back(tempitvl);
    }
    cout<<"Heuristic Score: ";
    cout<<score<<"\n";
    return partition;
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
bool nlrf(Packet& p1, Packet& p2){
    double wt1 = p1.deadline/(double)p1.penalty;
    double wt2 = p2.deadline/(double)p2.penalty;
    if(wt1 == wt2){
        return p1.arrival < p2.arrival;
    }
    return wt1 < wt2;
}


#define EDF 1
#define LRF 2
#define NLRF 3

void baseline(vector<Packet> packets, int T, int m, int opt){
    if(opt == EDF){
        sort(packets.begin(), packets.end(), edf);
    } else if(opt == LRF){
        sort(packets.begin(), packets.end(), lrf);
    } else {
        sort(packets.begin(), packets.end(), nlrf);
    }
    vector<queue<Packet>> transmissionQueue(m);
    vector<int> lastMission(m,0);
    int currentRU = 0;
    int score = 0;
    for(Packet& pckt: packets){
        bool assigned = false;
        //iterate through all the RU's until one of them is fit
        for(int ctr = 0; ctr < m; ctr++, currentRU = (currentRU + 1)%m){
            int prevEnd = lastMission[currentRU];
            double actualNewEnd = (double) max(prevEnd , pckt.arrival) + pckt.transmission_dur;
            int newEnd = (int)ceil(actualNewEnd);
            if(actualNewEnd <= pckt.deadline && newEnd < T){
                assigned = true;
                lastMission[currentRU] = newEnd;
                break;
            }
        }
        if(assigned){
            currentRU = (currentRU + 1 ) % m;
            score += pckt.penalty;
        }
    }
    if(opt == EDF){
        cout<<"EDF Score: "<<score<<"\n";
    } else if(opt == LRF){
        cout<<"LRF Score: "<<score<<"\n";
    } else {
        cout<<"NLRF Score: "<<score<<"\n";
    }
}
// ----------------------------------------------------------
//------------------- -------- ---------- -------------------
vector<pair<int, int>> findEdges(vector<Packet>& packets, vector<Interval> intervals){
    int packetCount = packets.size();
    int intervalCount = intervals.size();
    vector<pair<int, int>> edges;
    for(int i = 0; i < packetCount; i++){
        for(int j = 0; j < intervalCount; j++){
            int intervalStart = intervals[j].start;
            int intervalEnd = intervals[j].end;
            if(packetMatchesInterval(intervalStart, intervalEnd, packets[i])){
                edges.push_back({i, j+packetCount});
            }
        }
    }
    return edges;
}

// m matching solver
void matchingSolver(int m, vector<Packet>& packets, vector<Interval>& intervals){
    SmartDigraph g;
    SmartDigraph::ArcMap<int> cap(g, 1); //default value 1
    SmartDigraph::ArcMap<int> cost(g, 0); //default value 0

    int packetCount = packets.size();
    int intervalCount = intervals.size();
    int intervalStart = packetCount;

    //packets:
    for(int i = 0; i < packetCount; i++) g.addNode();
    //intervals
    for(int i = packetCount; i < packetCount+intervalCount; i++) g.addNode();

    vector<pair<int, int>> edges = findEdges(packets, intervals);

    //edges between packets and intervals:
    int edgesCount = edges.size();
    for(int i = 0; i < edgesCount; i++){
        SmartDigraph::Arc edge = g.addArc(g.nodeFromId(edges[i].first), g.nodeFromId(edges[i].second));
        
        cap[edge] = 1;
        cost[edge] = 0;
    }
    
    SmartDigraph::Node src = g.addNode();
    SmartDigraph::Node sink = g.addNode();

    int maxPenalty = 0;
    for(int i = 0; i < packetCount; i++){
        maxPenalty = max(maxPenalty, packets[i].penalty);
    }
    maxPenalty++;
    //edges between source and packets
    for(int i = 0; i < packetCount; i++){
        SmartDigraph::Arc edge = g.addArc(src, g.nodeFromId(i));
        cap[edge] = 1;
        cost[edge] = maxPenalty - packets[i].penalty;
    }

    //edges between intervals and sink
    for(int i = intervalStart; i < intervalStart + intervalCount; i++){
        SmartDigraph::Arc edge = g.addArc(g.nodeFromId(i), sink);
        cap[edge] = m;
        cost[edge] = 0;  
    }

    //Phase 1: run max flow algorithm to get max flow fstar (GoldBergs Preflow algorithm)
    Preflow<SmartDigraph> maxflowsolver(g, cap, src, sink);
    maxflowsolver.run();
    int fstar = maxflowsolver.flowValue();
    // cout<<"Fstar : "<<fstar<<"\n";

    int offsetCost = fstar * maxPenalty;

    //Phase 2: calculate min cost flow when fstar is known (Network Simplex)
    NetworkSimplex<SmartDigraph, int, int> ns(g);
    ns.costMap(cost).upperMap(cap).stSupply(src, sink, fstar);
    SmartDigraph::ArcMap<int> flows(g);
    //error handling
    NS::ProblemType status = ns.run();
    switch (status) {
    case NS::INFEASIBLE:
        cerr << "insufficient flow\n";
        break;
    case NS::OPTIMAL:
        ns.flowMap(flows);
        cout << "profit = " << offsetCost - ns.totalCost() << "\n"; 
        break;
    case NS::UNBOUNDED:
        cerr << "infinite flow\n";
        break;
    default:
        break;
    }
}


int main(){
    ios::sync_with_stdio(false);
    cin.tie(0);
    int T; cin>>T;
    while(T--){
        int m; cin>>m;
        int timeperiod; cin>>timeperiod;
        int packetCount; cin>>packetCount;
        vector<Packet> packets(0);
        for(int i = 0; i < packetCount; i++){
            int penalty, arri;
            double tdur;
            double ded;
            cin>>penalty>>arri>>tdur>>ded;
            Packet pckt(penalty, arri, tdur, ded, i);
            packets.push_back(pckt);
        }
        cout<<"---------\n";
        auto start = high_resolution_clock::now();
        vector<Interval> intervals = createInterval(packets, timeperiod, m);
        auto t1 = high_resolution_clock::now();
        baseline(packets, timeperiod, m, EDF);
        auto t2 = high_resolution_clock::now();
        baseline(packets, timeperiod, m, LRF);
        // baseline(packets, timeperiod, m, NLRF);
        // matchingSolver(m, packets, intervals);
        // auto t2 = high_resolution_clock::now();

        auto dur1 = duration_cast<microseconds>(t1 - start);
        auto dur2 = duration_cast<microseconds>(t2 - t1);

        // auto dur2 = duration_cast<microseconds>(t2 - t1);
        cout<<"Heuristic algo: "<<(dur1.count())/1000<<" milliseconds\n";
        cout<<"EDF algo: "<<(dur2.count())/1000<<" milliseconds\n";
        // cout<<"Matching algo: "<<dur2.count()<<" microseconds\n";
        cout<<"---------\n";
    }
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